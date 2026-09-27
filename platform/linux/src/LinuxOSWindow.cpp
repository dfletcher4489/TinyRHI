
#include "OSWindow.h"
#include "LinuxOSWindow.h"
#include <atomic>

#include <sys/mman.h>

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <poll.h>

static MPMCQueueData* freeList;
static int maxFreeListEntry = 0;

ALIGNAS(128) static std::atomic<int> boundedLinearAllocator{ 0 };
ALIGNAS(128) static std::atomic<size_t> enqueuePos{ 0 };
ALIGNAS(128) static std::atomic<size_t> dequeuePos{ 0 };

static int PopFromFreeList()
{
    MPMCQueueData* cell;

    size_t pos = dequeuePos.load(std::memory_order_relaxed);

    for (;;)
    {
        cell = &freeList[pos % maxFreeListEntry];
        size_t seq = cell->currentSequence.load(std::memory_order_acquire);
        intptr_t diff = (intptr_t)seq - (intptr_t)(pos + 1);
        if (diff == 0)
        {
            if (dequeuePos.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed, std::memory_order_relaxed))
                break;
        }
        else if (diff < 0)
            return -1;
        else
            pos = dequeuePos.load(std::memory_order_relaxed);
    }

    cell->currentSequence.store(pos + maxFreeListEntry, std::memory_order_release);

    int freeListIndex = cell->freeIndex;

    cell->freeIndex = -1;

    return freeListIndex;
}

static int FindFreeIndex()
{
    int ret = PopFromFreeList();

    if (ret < 0)
    {
        int linearTop = boundedLinearAllocator.load(std::memory_order_acquire);

        while (linearTop < maxFreeListEntry)
        {
            if (boundedLinearAllocator.compare_exchange_weak(linearTop, linearTop + 1, std::memory_order_relaxed, std::memory_order_relaxed))
            {
                ret = linearTop;
                break;
            }
        }
    }

    return ret;
}

static GenericWindowEventPacked* GetWindowEventPacked(GenericWindowEventBuffer* buffer)
{
    size_t currentWrite = buffer->dataWrite.load(std::memory_order_relaxed);
    size_t currentRead = buffer->dataRead.load(std::memory_order_relaxed);

    void* currentWritePtr = (void*)((uintptr_t)buffer->dataHead + (currentWrite & (buffer->dataSize - 1)));

    if ((currentWrite-currentRead) >= buffer->dataSize)
    {
        buffer->ringLapsSinceLastRead.fetch_add(1, std::memory_order_relaxed);
    }

    return (GenericWindowEventPacked*)(currentWritePtr);
}

static void CommitWindowEventPacked(GenericWindowEventBuffer* buffer)
{
    buffer->dataWrite.fetch_add(sizeof(GenericWindowEventPacked), std::memory_order_release);
}

static int PumpWindowEventsPacked(GenericWindowEventBuffer* buffer, GenericWindowInfo* info)
{
    int packedEventsCount = 0;

    size_t writeHead = buffer->dataWrite.load(std::memory_order_acquire);
    size_t readPos = buffer->dataRead.load(std::memory_order_relaxed);

    while (readPos < writeHead)
    {
        GenericWindowEventPacked* currentPacked = (GenericWindowEventPacked*)((uintptr_t)buffer->dataHead + (readPos & (buffer->dataSize - 1)));

        switch (currentPacked->EventType)
        {
        case WINDOW_EVENT_TYPE_MOUSE_LEFT_BUTTON:
            info->clicked = currentPacked->EventPacked;
            break;
        case WINDOW_EVENT_TYPE_RESIZE_REQUESTED:
            info->resizeRequested = currentPacked->EventPacked;
            break;
        case WINDOW_EVENT_TYPE_SHOULD_BE_CLOSED:
            info->shouldBeClosed = currentPacked->EventPacked;
            break;
        case WINDOW_EVENT_TYPE_WINDOW_SIZE:
            info->width = GET_WINDOW_SIZE_EVENT_WIDTH(currentPacked->EventPacked);
            info->height = GET_WINDOW_SIZE_EVENT_HEIGHT(currentPacked->EventPacked);
            info->maximized = GET_WINDOW_SIZE_EVENT_MAXIMIZED(currentPacked->EventPacked);
            info->minimized = GET_WINDOW_SIZE_EVENT_MINIMIZED(currentPacked->EventPacked);
            break;
        case WINDOW_EVENT_TYPE_MOUSE_COORDINATES:
            info->currentCursorX = GET_WINDOW_COORDINATES_EVENT_X(currentPacked->EventPacked);
            info->currentCursorY = GET_WINDOW_COORDINATES_EVENT_Y(currentPacked->EventPacked);
            break;
        case WINDOW_EVENT_TYPE_KEY_ACTION:
            info->actions[GET_KEY_CODE(currentPacked->EventPacked)].Update(GET_KEY_ACTION(currentPacked->EventPacked));
            break;
        }

        readPos += sizeof(GenericWindowEventPacked);
        packedEventsCount++;
    }

    buffer->dataRead.store(readPos, std::memory_order_release);

    if (buffer->ringLapsSinceLastRead.load(std::memory_order_acquire))
        packedEventsCount = -packedEventsCount;

    buffer->ringLapsSinceLastRead.store(0, std::memory_order_relaxed);

    return packedEventsCount;
}

int OSWindowSeedEventBuffer(OSWindow* window, void* bufferMemory, size_t bufferSize)
{
    if (bufferSize & (bufferSize - 1))
    {
        return -1;
    }

    window->eventBuffer.dataHead = bufferMemory;
    window->eventBuffer.dataSize = bufferSize;
    window->eventBuffer.dataRead = 0;
    window->eventBuffer.dataWrite = 0;
    window->eventBuffer.ringLapsSinceLastRead = 0;
    window->eventBuffer.requestedFullScreen = 0;

    return 0;
}

#if defined(WINDOW_USE_WAYLAND)

#define USE_BUFFER
#define WINDOW_HEADER_TEXT_MAX_LEN 32

#include <wayland-client.h>
#include <xdg-shell-client-protocol.h>
#include <xdg-shell-protocol.c>
#include <xdg-decoration-client-protocol.h>
#include <xdg-decoration-client-protocol.c>

struct OSWaylandData
{
    struct wl_surface* surface;
    struct xdg_surface* xdg_surface;
    struct xdg_toplevel* xdg_toplevel;
    struct zxdg_toplevel_decoration_v1 *decoration;
    GenericWindowEventBuffer* windowEventBuffer;
    size_t surfaceConfigured;
    char windowHeader[WINDOW_HEADER_TEXT_MAX_LEN]; 
    int width;
    int height;
#ifdef USE_BUFFER
    struct wl_buffer* buffer;
    struct wl_shm_pool* pool;
    void* shmdata;
    size_t bufferSize;
    int bufferFile;
    int pad;
#endif
};

static int initialize = 0;
static struct wl_display *display = NULL;
static struct wl_compositor *compositor = NULL;
static struct xdg_wm_base* xdg_wm_base = NULL;
static struct zxdg_decoration_manager_v1* decoration_manager = NULL;

#ifdef USE_BUFFER
static struct wl_shm *shm = NULL;
#endif

static OSWaylandData* instancePointers;

static void CleanOSWaylandData(OSWaylandData* data)
{
    data->surface = NULL;
    data->xdg_surface = NULL;
    data->xdg_toplevel = NULL;
    data->windowEventBuffer = NULL;
    data->surfaceConfigured = 0;
    data->decoration = NULL;
#ifdef USE_BUFFER
    data->buffer = NULL;
    data->pool = NULL;
    data->shmdata = NULL;
    data->bufferSize = 0;
    data->bufferFile = -1;
#endif
}

static void ReturnIndex(int index)
{
    MPMCQueueData* cell;

    size_t pos = enqueuePos.load(std::memory_order_relaxed);

    for (;;)
    {
        cell = &freeList[pos % maxFreeListEntry];
        size_t seq = cell->currentSequence.load(std::memory_order_acquire);
        intptr_t diff = (intptr_t)seq - (intptr_t)pos;
        if (diff == 0)
        {
            if (enqueuePos.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed))
                break;
        }
        else if (diff < 0)
            return;
        else
            pos = enqueuePos.load(std::memory_order_relaxed);
    }

    CleanOSWaylandData(&instancePointers[index]);

    cell->freeIndex = index;
    cell->currentSequence.store(pos + 1, std::memory_order_release);
}

static void
RegistryGlobalHandler(
    void *data,
    struct wl_registry *registry,
    uint32_t name,
    const char *interface,
    uint32_t version)
{

    if (strcmp(interface, "wl_compositor") == 0) 
    {
        compositor = (wl_compositor*)wl_registry_bind(registry, name, &wl_compositor_interface, 4);
    } 
#ifdef USE_BUFFER
    else if (strcmp(interface, "wl_shm") == 0) 
    {
        shm = (wl_shm*)wl_registry_bind(registry, name, &wl_shm_interface, 1);
    }
#endif
    else if (strcmp(interface, xdg_wm_base_interface.name) == 0) 
    {
        xdg_wm_base = (struct xdg_wm_base*)wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
    }
    else if (strcmp(interface, zxdg_decoration_manager_v1_interface.name) == 0) 
    {
        decoration_manager = (zxdg_decoration_manager_v1*)wl_registry_bind(registry, name, &zxdg_decoration_manager_v1_interface, 1);
    }
}

static void RegistryGlobalRemoveHandler
(
    void *data,
    struct wl_registry *registry,
    uint32_t name
) 
{
    printf("removed: %u\n", name);
}

static void XdgWmBasePing(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial)
{
    xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = XdgWmBasePing,
};

static void XdgSurfaceConfigure(void *data, struct xdg_surface *xdg_surface, uint32_t serial)
{
    xdg_surface_ack_configure(xdg_surface, serial);
    OSWaylandData* osData = (OSWaylandData*)data;
    osData->surfaceConfigured = true;
}

static const struct xdg_surface_listener xdg_surface_listener = 
{
    .configure = XdgSurfaceConfigure,
};

static void XdgToplevelConfigure(void *data, struct xdg_toplevel *toplevel, int32_t width, int32_t height, struct wl_array *states) 
{

}

static void XdgToplevelClose(void *data, struct xdg_toplevel *toplevel) 
{ 

}

static const struct xdg_toplevel_listener xdg_toplevel_listener = 
{
    .configure = XdgToplevelConfigure,
    .close = XdgToplevelClose,
};



static int InitializeWayland()
{
    if (initialize)
    {
        return OS_WINDOW_CREATE_FAILED;
    }

    display = wl_display_connect(NULL);

    struct wl_registry *registry = wl_display_get_registry(display);

    struct wl_registry_listener registry_listener = 
    {
        .global = RegistryGlobalHandler,
        .global_remove = RegistryGlobalRemoveHandler
    };

    wl_registry_add_listener(registry, &registry_listener, NULL);

    wl_display_roundtrip(display);

    if (!compositor || !xdg_wm_base || !decoration_manager)
    {
        printf("Missing required globals\n");
        return OS_WINDOW_CREATE_FAILED;
    }

#ifdef USE_BUFFER
    if (!shm)
    {
        printf("Missing required globals\n");
        return OS_WINDOW_CREATE_FAILED;
    }
#endif

    xdg_wm_base_add_listener(xdg_wm_base, &xdg_wm_base_listener, NULL);

    wl_registry_destroy(registry);

    initialize = 1;

    return OS_WINDOW_SUCCESS;
}

OSWindowMemoryRequirements OSGetWindowMemoryRequirements(int maxNumberOfWindows)
{
    int windowDataSize = (maxNumberOfWindows) * sizeof(OSWaylandData);
    int freeListSize = (maxNumberOfWindows) * sizeof(MPMCQueueData);

    OSWindowMemoryRequirements memReqs{ windowDataSize + freeListSize, alignof(OSWaylandData) };

    return memReqs;
}

int OSSeedWindowMemory(void* dataSource, int dataSize, int maxNumberOfWindows)
{
    uintptr_t dataHead = (uintptr_t)dataSource;

    int handleSize = maxNumberOfWindows;

    instancePointers = (OSWaylandData*)dataHead;

    dataHead += handleSize * sizeof(OSWaylandData);

    freeList = (MPMCQueueData*)dataHead;

    for (int i = 0; i < handleSize; i++)
    {
        freeList[i].currentSequence.store(i, std::memory_order_relaxed);

        CleanOSWaylandData(&instancePointers[i]);
    }

    maxFreeListEntry = handleSize;

    return OS_WINDOW_SUCCESS;
}

void CloseAllWindows()
{
    for (int idx = 0; idx < maxFreeListEntry; idx++)
    {
        if (instancePointers[idx].surface)
        {
#ifdef USE_BUFFER
            if (instancePointers[idx].buffer)
            {
                wl_buffer_destroy(instancePointers[idx].buffer);
            }

            if (instancePointers[idx].pool)
            {
                wl_shm_pool_destroy(instancePointers[idx].pool);
            }

            if (instancePointers[idx].shmdata)
            {
                munmap(instancePointers[idx].shmdata, instancePointers[idx].bufferSize);
            }

            if (instancePointers[idx].bufferFile >= 0)
            {
                close(instancePointers[idx].bufferFile);
            }
#endif
            wl_surface_destroy(instancePointers[idx].surface);
        }

        CleanOSWaylandData(&instancePointers[idx]);

        freeList[idx].currentSequence.store(idx, std::memory_order_relaxed);
    }

    enqueuePos.store(0, std::memory_order_relaxed);
    dequeuePos.store(0, std::memory_order_relaxed);
    boundedLinearAllocator.store(0, std::memory_order_relaxed);

    if (initialize)
    {
        zxdg_decoration_manager_v1_destroy(decoration_manager);
        xdg_wm_base_destroy(xdg_wm_base);
#ifdef USE_BUFFER
        wl_shm_destroy(shm);
#endif
        wl_compositor_destroy(compositor);
        wl_display_disconnect(display);
        initialize = 0;
    }
}

int OSCreateWindow(const char* name, int requestedDimensionX, int requestDimensionY, OSWindow* windowData)
{
    if (!initialize)
    {
        int retCode = InitializeWayland();

        if (retCode)
        {
            return OS_WINDOW_CREATE_FAILED;
        }
    }

    struct wl_surface *surface = NULL;

    surface = wl_compositor_create_surface(compositor);

    if (!surface)
    {
        return OS_WINDOW_CREATE_FAILED;
    }

    int osIndex = FindFreeIndex();

    OSWaylandData* data = &instancePointers[osIndex];

    CleanOSWaylandData(data);

    data->surface = surface;
    data->surfaceConfigured = 0; 
    data->windowEventBuffer = &windowData->eventBuffer;
    data->width = requestedDimensionX;
    data->height = requestDimensionY;

    memcpy(data->windowHeader, name, strnlen(name, WINDOW_HEADER_TEXT_MAX_LEN));

    wl_surface_set_user_data(surface, data);

    windowData->internalOSHandle = osIndex;

    return OS_WINDOW_SUCCESS;
}

int OSWindowPollEvents(OSWindow* window, GenericWindowInfo* info)
{
    int ret = OS_WINDOW_SUCCESS;

    wl_display_dispatch_pending(display);
    wl_display_flush(display);

    return ret;
}

int OSWindowGetInternalData(OSWindow* window, void* internalDataStruct)
{
    return OS_WINDOW_SUCCESS;
}

int OSWindowSetText(OSWindow* window, const char* text)
{
    OSWaylandData* data = &instancePointers[window->internalOSHandle];

    int count = strnlen(text, WINDOW_HEADER_TEXT_MAX_LEN);

    memcpy(data->windowHeader, text, count);

    data->windowHeader[count] = '\0';

    xdg_toplevel_set_title(data->xdg_toplevel, data->windowHeader);

    return OS_WINDOW_SUCCESS;
}

int OSWindowHide(OSWindow* window)
{
    OSWaylandData* data = &instancePointers[window->internalOSHandle];

    if (!data->xdg_surface || !data->xdg_toplevel)
    {
        return OS_WINDOW_SUCCESS;
    }

    zxdg_toplevel_decoration_v1_destroy(data->decoration);
    xdg_toplevel_destroy(data->xdg_toplevel);
    xdg_surface_destroy(data->xdg_surface);

    data->xdg_surface = NULL;
    data->xdg_toplevel = NULL;
    data->decoration = NULL;
    data->surfaceConfigured = 0;

    wl_surface_attach(data->surface, NULL, 0, 0);
    wl_surface_commit(data->surface);

    return OS_WINDOW_SUCCESS;
}

int OSWindowShow(OSWindow* window)
{
    OSWaylandData* data = &instancePointers[window->internalOSHandle];

    if (data->xdg_surface || data->xdg_toplevel)
    {
        return OS_WINDOW_SUCCESS;
    }

    struct xdg_surface *xdg_surface = xdg_wm_base_get_xdg_surface(xdg_wm_base, data->surface);
    
    struct xdg_toplevel *xdg_toplevel = xdg_surface_get_toplevel(xdg_surface);

    struct zxdg_toplevel_decoration_v1 *decoration = zxdg_decoration_manager_v1_get_toplevel_decoration(decoration_manager, xdg_toplevel);

    data->xdg_surface = xdg_surface;
    data->xdg_toplevel = xdg_toplevel;
    data->decoration = decoration;
    
    zxdg_toplevel_decoration_v1_set_mode(decoration, ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);

    xdg_surface_add_listener(xdg_surface, &xdg_surface_listener, data);

    xdg_toplevel_add_listener(xdg_toplevel, &xdg_toplevel_listener, NULL);

    xdg_toplevel_set_title(data->xdg_toplevel, data->windowHeader);
    
    wl_surface_commit(data->surface); 
    wl_display_flush(display);
    
    while (!data->surfaceConfigured) 
    {
        wl_display_dispatch(display);    
    }

    if (data->buffer)
    {
        wl_surface_attach(data->surface, data->buffer, 0, 0);
        wl_surface_damage_buffer(data->surface, 0, 0, data->width, data->height);
        wl_surface_commit(data->surface);
        wl_display_flush(display);
    }

    return OS_WINDOW_SUCCESS;
}

int OSWindowAttachBuffer(OSWindow* window, void** bufferData, size_t bufferSize, int width, int height)
{
#ifdef USE_BUFFER
    OSWaylandData* data = &instancePointers[window->internalOSHandle];

    data->bufferFile = memfd_create("bufferPool", 0);

    data->bufferSize = bufferSize;

    ftruncate(data->bufferFile, bufferSize);

    data->shmdata = mmap(NULL, bufferSize, PROT_READ | PROT_WRITE, MAP_SHARED, data->bufferFile, 0);

    *bufferData = data->shmdata;

    data->pool = wl_shm_create_pool(shm, data->bufferFile, data->bufferSize);

    data->buffer = wl_shm_pool_create_buffer(data->pool, 0, width, height, width*sizeof(uint32_t), WL_SHM_FORMAT_ARGB8888);

    wl_surface_attach(data->surface, data->buffer, 0, 0);
    wl_surface_damage_buffer(data->surface, 0, 0, width, height);
    wl_surface_commit(data->surface);
    wl_display_flush(display);
#endif
    return OS_WINDOW_SUCCESS;
}

#elif defined(WINDOW_USE_X11)

static int FindFreeIndex()
{
    for (int i = 0; i < maxFreeListEntry; i++)
    {
       
    }
    return -1;
}

OSWindowMemoryRequirements OSGetWindowMemoryRequirements(int maxNumberOfWindows)
{
    OSWindowMemoryRequirements memReqs{ 0, alignof(void*) };

    return memReqs;
}

void CloseAllWindows()
{
    for (int i = 0; i < maxFreeListEntry; i++)
    {
       
    }
}

int OSSeedWindowMemory(void* dataSource, int dataSize, int maxNumberOfWindows)
{
    return 0;
}

int OSCreateWindow(const char* name, int requestedDimensionX, int requestDimensionY, OSWindow* windowData)
{
    return 0;
}

int PollOSWindowEvents(OSWindow* window)
{
    int ret = 0;

    return ret;
}

int GetInternalOSData(OSWindow* window, void* internalDataStruct)
{
    return 0;
}

int SetOSWindowText(OSWindow* window, const char* text)
{
    return 0;
}

#endif