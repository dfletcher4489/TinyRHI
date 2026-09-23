
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

#if defined(WINDOW_USE_WAYLAND)

#include <wayland-client.h>
#include <xdg-shell-client-protocol.h>
#include <xdg-shell-protocol.c>
#include <xdg-decoration-client-protocol.h>
#include <xdg-decoration-client-protocol.c>

struct pool_data {
    int fd;
    unsigned capacity;
    unsigned size;
};

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

    //windowPtrs[index] = NULL;
    //instancePointers[index] = NULL;
    cell->freeIndex = index;
    cell->currentSequence.store(pos + 1, std::memory_order_release);
}

OSWindowMemoryRequirements OSGetWindowMemoryRequirements(int maxNumberOfWindows)
{
    int handlesSize = (maxNumberOfWindows) * 0;
    int handlesWndSize = (maxNumberOfWindows) * 0;
    int freeListSize = (maxNumberOfWindows) * sizeof(MPMCQueueData);

    OSWindowMemoryRequirements memReqs{ handlesSize + handlesWndSize + freeListSize, alignof(uintptr_t) };

    return memReqs;
}

static struct wl_display *display = NULL;
static struct wl_compositor *compositor = NULL;
static struct wl_shm *shm = NULL;
static struct wl_surface *surface = NULL;
static struct wl_shm_pool *pool = NULL;
static struct wl_buffer *buffer;
static struct pool_data data;
static int memfd = -1;
static void *shm_data; 
static struct xdg_wm_base* xdg_wm_base;
static struct zxdg_decoration_manager_v1* decoration_manager;

static char framebuffer[1 * 1024 * 1024];

static void RegistryGlobalHandler
(
    void *data,
    struct wl_registry *registry,
    uint32_t name,
    const char *interface,
    uint32_t version
) {

    if (strcmp(interface, "wl_compositor") == 0) {
        compositor = (wl_compositor*)wl_registry_bind(registry, name,
            &wl_compositor_interface, 4);
    } else if (strcmp(interface, "wl_shm") == 0) {
        shm = (wl_shm*)wl_registry_bind(registry, name,
            &wl_shm_interface, 1);
    }
    else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
        xdg_wm_base = (struct xdg_wm_base*)wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
    }
    else if (strcmp(interface, zxdg_decoration_manager_v1_interface.name) == 0) {
        decoration_manager = (zxdg_decoration_manager_v1*)wl_registry_bind(
            registry, name, &zxdg_decoration_manager_v1_interface, 1);
    }


   // printf("interface: '%s', version: %u, name: %u\n", interface, version, name);
}

void RegistryGlobalRemoveHandler
(
    void *data,
    struct wl_registry *registry,
    uint32_t name
) 
{
    printf("removed: %u\n", name);
}

void CloseAllWindows()
{
    zxdg_decoration_manager_v1_destroy(decoration_manager);

    wl_buffer_destroy(buffer);

    wl_surface_destroy(surface);

    wl_shm_pool_destroy(pool);

    munmap(shm_data, 1024 * 1024);

    close(memfd);

    xdg_wm_base_destroy(xdg_wm_base);
    wl_shm_destroy(shm);
    wl_compositor_destroy(compositor);
    wl_display_disconnect(display);
}


void hello_create_memory_pool()
{
    memfd = memfd_create("whatever", 0);

    ftruncate(memfd , 1024*1024);

    data.capacity = 1024 * 1024;
    data.size = 0;
    data.fd = memfd;

    shm_data = mmap(NULL, 1024*1024, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, 0);

    pool = wl_shm_create_pool(shm, data.fd, data.capacity);
}

static void XdgWmBasePing(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial)
{
    xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = XdgWmBasePing,
};

int OSSeedWindowMemory(void* dataSource, int dataSize, int maxNumberOfWindows)
{
    if (!display)
    {
        display = wl_display_connect(NULL);

        struct wl_registry *registry = wl_display_get_registry(display);

        struct wl_registry_listener registry_listener = {
            .global = RegistryGlobalHandler,
            .global_remove = RegistryGlobalRemoveHandler
        };

        wl_registry_add_listener(registry, &registry_listener, NULL);

        wl_display_roundtrip(display);

        if (!compositor || !shm || !xdg_wm_base || !decoration_manager) {
            printf("Missing required globals: decomanager\n",
            (void*)decoration_manager);
             fflush(stdout);
        }

        xdg_wm_base_add_listener(xdg_wm_base, &xdg_wm_base_listener, NULL);

        wl_registry_destroy(registry);

        hello_create_memory_pool();
    }

    return 0;
}

static const uint32_t PIXEL_FORMAT_ID = WL_SHM_FORMAT_ARGB8888;

struct wl_buffer *hello_create_buffer(struct wl_shm_pool *pool,
    unsigned width, unsigned height)
{
    buffer = wl_shm_pool_create_buffer(pool,
        data.size, width, height,
        width*sizeof(uint32_t), PIXEL_FORMAT_ID);

    if (buffer == NULL)
    {
        data.size += width*height*sizeof(uint32_t);
    }

    return buffer;
}

static bool surface_configured = false;

static void XdgSurfaceConfigure(void *data, struct xdg_surface *xdg_surface, uint32_t serial)
{
    xdg_surface_ack_configure(xdg_surface, serial);
    surface_configured = true;
}

static const struct xdg_surface_listener xdg_surface_listener = {
    .configure = XdgSurfaceConfigure,
};

static void XdgToplevelConfigure(void *data, struct xdg_toplevel *toplevel,
                                  int32_t width, int32_t height, struct wl_array *states) {}
static void XdgToplevelClose(void *data, struct xdg_toplevel *toplevel) { /* set running=false */ }

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = XdgToplevelConfigure,
    .close = XdgToplevelClose,
};

int OSCreateWindow(const char* name, int requestedDimensionX, int requestDimensionY, OSWindow* windowData)
{
    surface = wl_compositor_create_surface(compositor);

    if (surface == NULL)
    {
        return -1;
    }

    struct xdg_surface *xdg_surface = xdg_wm_base_get_xdg_surface(xdg_wm_base, surface);
    struct xdg_toplevel *xdg_toplevel = xdg_surface_get_toplevel(xdg_surface);

    xdg_toplevel_set_title(xdg_toplevel, name);

    struct zxdg_toplevel_decoration_v1 *decoration = zxdg_decoration_manager_v1_get_toplevel_decoration(decoration_manager, xdg_toplevel);
    
    zxdg_toplevel_decoration_v1_set_mode(decoration, ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);

    xdg_surface_add_listener(xdg_surface, &xdg_surface_listener, NULL);

    xdg_toplevel_add_listener(xdg_toplevel, &xdg_toplevel_listener, NULL);

    wl_surface_set_user_data(surface, NULL);

    wl_surface_commit(surface); 
    wl_display_flush(display);
    
    while (!surface_configured) 
    {
        wl_display_dispatch(display);    
    }

    buffer = hello_create_buffer(pool, requestedDimensionX, requestDimensionY);

    memset(shm_data, 0xFF, requestDimensionY * requestedDimensionX * sizeof(uint32_t));
    
    wl_surface_attach(surface, buffer, 0, 0);
    wl_surface_damage_buffer(surface, 0, 0, requestedDimensionX, requestDimensionY);
    wl_surface_commit(surface);
    wl_display_flush(display);

    return 0;
}

int OSWindowPollEvents(OSWindow* window, GenericWindowInfo* info)
{
    int ret = 0;

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
    return OS_WINDOW_SUCCESS;
}

int OSWindowShow(OSWindow* window)
{
    return OS_WINDOW_SUCCESS;
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