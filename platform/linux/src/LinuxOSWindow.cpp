
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

#if defined(WINDOW_USE_WAYLAND) || 1

//#define USE_BUFFER
#define WINDOW_HEADER_TEXT_MAX_LEN 32

#include <wayland-client.h>
#include <linux/input-event-codes.h>
#include <sys/mman.h>
#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-keysyms.h>
#include "xdg-shell-client-protocol.h"
#include "xdg-decoration-client-protocol.h"

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
static struct wl_seat* seat = NULL;
static struct wl_keyboard* keyboard = NULL;
static struct wl_pointer* pointer = NULL;
static struct xkb_context* xkb_ctx = NULL;
static xkb_keymap* keymap = NULL; 
static xkb_state* keystate = NULL;
static int pointerActiveSurface = -1;
static int keyboardActiveSurface = -1;

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
    else if (strcmp(interface, wl_seat_interface.name) == 0)
    {
        seat = (wl_seat*)wl_registry_bind(registry, name, &wl_seat_interface, version < 5 ? version : 5);
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
    OSWaylandData* wayLandData = (OSWaylandData*)data;

    printf("width=%d, height=%d\n", width, height);

    if (wayLandData->surfaceConfigured == 1)
    {
        int minimized = 0;
        int activated = 0;
        int resizing = 0;
        int maximized = 0;

        const uint32_t* begin = static_cast<const uint32_t*>(states->data);
        const uint32_t* end   = begin + (states->size / sizeof(uint32_t));

        for (const uint32_t* s = begin; s < end; ++s)
        {
            printf("%d\n", *s);
            switch (*s)
            {
                case XDG_TOPLEVEL_STATE_ACTIVATED: activated = 1; break;
                case XDG_TOPLEVEL_STATE_SUSPENDED: minimized = 1; break;
                case XDG_TOPLEVEL_STATE_RESIZING: resizing = 1; break;
                case XDG_TOPLEVEL_STATE_MAXIMIZED: maximized = 1; break;
                case XDG_TOPLEVEL_STATE_FULLSCREEN: break;
                default: break;
            }
        }

        if (resizing || minimized || maximized)
        {
            printf("resized width=%d, height=%d\n", width, height);

            GenericWindowEventPacked* packed = GetWindowEventPacked(wayLandData->windowEventBuffer);
    
            packed->EventType = WINDOW_EVENT_TYPE_WINDOW_SIZE;
            packed->EventPacked = PACK_WINDOW_SIZE_EVENT(width, height, minimized, maximized);

            CommitWindowEventPacked(wayLandData->windowEventBuffer);

            if (!minimized)
            {
                packed = GetWindowEventPacked(wayLandData->windowEventBuffer);
                packed->EventType = WINDOW_EVENT_TYPE_RESIZE_REQUESTED;
                packed->EventPacked = 1;
                CommitWindowEventPacked(wayLandData->windowEventBuffer);
            }
        }
    }
}

static void XdgToplevelClose(void *data, struct xdg_toplevel *toplevel) 
{ 
    OSWaylandData* wayLandData = (OSWaylandData*)data;

    GenericWindowEventPacked* packed = GetWindowEventPacked(wayLandData->windowEventBuffer);
    
    packed->EventType = WINDOW_EVENT_TYPE_SHOULD_BE_CLOSED;
    packed->EventPacked = 1;

    CommitWindowEventPacked(wayLandData->windowEventBuffer);
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = 
{
    .configure = XdgToplevelConfigure,
    .close = XdgToplevelClose,
};

static void KbKeymap(void *data, struct wl_keyboard *kb, uint32_t format, int fd, uint32_t size)
{
    char *map = (char*)mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);

    if (!xkb_ctx)
    {
        xkb_ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    }

    if (keymap)
    {
        xkb_keymap_unref(keymap);
    }

    keymap = xkb_keymap_new_from_string(xkb_ctx, map, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);

    munmap(map, size);
    close(fd);

    if (keystate)
    {
        xkb_state_unref(keystate);
    }

    keystate = xkb_state_new(keymap);
}

static void KbEnter(void *d, struct wl_keyboard *kb, uint32_t serial, struct wl_surface *surface, struct wl_array *keys) 
{
    int i = 0;
    for (i = 0; i < maxFreeListEntry; i++)
    {
        if (instancePointers[i].surface == surface)
        {
            keyboardActiveSurface = i;
            break;
        }
    }

    if (i == maxFreeListEntry)
    {
        printf("Unregistered surface attempted to acquire keyboard\n");
    }
}

static void KbLeave(void *d, struct wl_keyboard *kb, uint32_t serial, struct wl_surface *surface) 
{
    int i = 0;
    for (i = 0; i < maxFreeListEntry; i++)
    {
        if (instancePointers[i].surface == surface)
        {
            keyboardActiveSurface = i;
            break;
        }
    }

    if (i == maxFreeListEntry)
    {
        printf("Unregistered surface lost keyboard\n");
    }

    keyboardActiveSurface = -1;
}

static void KbKey(void *d, struct wl_keyboard *kb, uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
    if (keyboardActiveSurface < 0)
    {
        return;
    }

    OSWaylandData* wayData = &instancePointers[keyboardActiveSurface];
    xkb_keycode_t code = key + 8;   // Wayland sends evdev codes; xkb offsets by 8

    xkb_layout_index_t layout = xkb_state_key_get_layout(keystate, code);
    const xkb_keysym_t *syms;
    int keyCount = xkb_keymap_key_get_syms_by_level(keymap, code, layout, 0, &syms);
    
    bool pressed = (state == WL_KEYBOARD_KEY_STATE_PRESSED);
    bool released = (state == WL_KEYBOARD_KEY_STATE_RELEASED);

   // printf("keycode=%d, sym=%d, pressed=%d, release=%d active=%d\n", code, *syms, pressed, released, keyboardActiveSurface);

    int keyCode = 0;
    int keyAction = released ? RELEASED : PRESSED;

    switch (syms[0])  
    {
    case XKB_KEY_0:
        keyCode = KC_ZERO;
        break;
    case XKB_KEY_1:
        keyCode = KC_ONE;
        break;
    case XKB_KEY_2:
        keyCode = KC_TWO;
        break;
    case XKB_KEY_3:
        keyCode = KC_THREE;
        break;
    case XKB_KEY_4:
        keyCode = KC_FOUR;
        break;
    case XKB_KEY_5:
        keyCode = KC_FIVE;
        break;
    case XKB_KEY_6:
        keyCode = KC_SIX;
        break;
    case XKB_KEY_7:
        keyCode = KC_SEVEN;
        break;
    case XKB_KEY_8:
        keyCode = KC_EIGHT;
        break;
    case XKB_KEY_9:
        keyCode = KC_NINE;
        break;

    case XKB_KEY_a:
        keyCode = KC_A;
        break;
    case XKB_KEY_b:
        keyCode = KC_B;
        break;
    case XKB_KEY_c:
        keyCode = KC_C;
        break;
    case XKB_KEY_d:
        keyCode = KC_D;
        break;
    case XKB_KEY_e:
        keyCode = KC_E;
        break;
    case XKB_KEY_f:
        keyCode = KC_F;
        break;
    case XKB_KEY_g:
        keyCode = KC_G;
        break;
    case XKB_KEY_h:
        keyCode = KC_H;
        break;
    case XKB_KEY_i:
        keyCode = KC_I;
        break;
    case XKB_KEY_j:
        keyCode = KC_J;
        break;
    case XKB_KEY_k:
        keyCode = KC_K;
        break;
    case XKB_KEY_l:
        keyCode = KC_L;
        break;
    case XKB_KEY_m:
        keyCode = KC_M;
        break;
    case XKB_KEY_n:
        keyCode = KC_N;
        break;
    case XKB_KEY_o:
        keyCode = KC_O;
        break;
    case XKB_KEY_p:
        keyCode = KC_P;
        break;
    case XKB_KEY_q:
        keyCode = KC_Q;
        break;
    case XKB_KEY_r:
        keyCode = KC_R;
        break;
    case XKB_KEY_s:
        keyCode = KC_S;
        break;
    case XKB_KEY_t:
        keyCode = KC_T;
        break;
    case XKB_KEY_u:
        keyCode = KC_U;
        break;
    case XKB_KEY_v:
        keyCode = KC_V;
        break;
    case XKB_KEY_w:
        keyCode = KC_W;
        break;
    case XKB_KEY_x:
        keyCode = KC_X;
        break;
    case XKB_KEY_y:
        keyCode = KC_Y;
        break;
    case XKB_KEY_z:
        keyCode = KC_Z;
        break;

    case XKB_KEY_F1:
        keyCode = KC_F1;
        break;
    case XKB_KEY_F2:
        keyCode = KC_F2;
        break;
    case XKB_KEY_F3:
        keyCode = KC_F3;
        break;
    case XKB_KEY_F4:
        keyCode = KC_F4;
        break;
    case XKB_KEY_F5:
        keyCode = KC_F5;
        break;
    case XKB_KEY_F12:
        keyCode = KC_F12;
        break;

    case XKB_KEY_Escape:
        keyCode = KC_ESC;
        break;
    case XKB_KEY_Tab:
        keyCode = KC_TAB;
        break;
    case XKB_KEY_ISO_Left_Tab:
        keyCode = KC_TAB;
        break; 
    case XKB_KEY_Shift_L:
        keyCode = KC_LSHIFT;
        break;
    case XKB_KEY_Shift_R:
        keyCode = KC_LSHIFT;
        break; 
    case XKB_KEY_Control_L:
        keyCode = KC_LCTRL;
        break;
    case XKB_KEY_Control_R:
        keyCode = KC_LCTRL;
        break; 
    case XKB_KEY_Alt_L:
        keyCode = KC_LALT;
        break;
    case XKB_KEY_Alt_R:
        keyCode = KC_LALT;
        break; 
    case XKB_KEY_space:
        keyCode = KC_SPACE;
        break;
    case XKB_KEY_Return:
        keyCode = KC_ENTER;
        break;
    case XKB_KEY_BackSpace:
        keyCode = KC_BACKSPACE;
        break;

    case XKB_KEY_Up:
        keyCode = KC_UP;
        break;
    case XKB_KEY_Down:
        keyCode = KC_DOWN;
        break;
    case XKB_KEY_Left:
        keyCode = KC_LEFT;
        break;
    case XKB_KEY_Right:
        keyCode = KC_RIGHT;
        break;
    case XKB_KEY_Insert:
        keyCode = KC_INSERT;
        break;
    case XKB_KEY_Delete:
        keyCode = KC_DELETE;
        break;
    case XKB_KEY_Home:
        keyCode = KC_HOME;
        break;
    case XKB_KEY_End:
        keyCode = KC_END;
        break;
    case XKB_KEY_Page_Up:
        keyCode = KC_PAGEUP;
        break;
    case XKB_KEY_Page_Down:
        keyCode = KC_PAGEDOWN;
        break;
    default:
        break;
    }

    GenericWindowEventPacked* packed = GetWindowEventPacked(wayData->windowEventBuffer);
    packed->EventType = WINDOW_EVENT_TYPE_KEY_ACTION;
    packed->EventPacked = PACK_KEY_CODE_ACTION(keyCode, keyAction);
    CommitWindowEventPacked(wayData->windowEventBuffer);
}

static void KbModifiers(void *d, struct wl_keyboard *kb, uint32_t serial,
                        uint32_t depressed, uint32_t latched, uint32_t locked, uint32_t group)
{
    xkb_state_update_mask(keystate, depressed, latched, locked, 0, 0, group);
}

static void KbRepeatInfo(void *d, struct wl_keyboard *kb, int32_t rate, int32_t delay) 
{

}

static const struct wl_keyboard_listener keyboard_listener = {
    KbKeymap, KbEnter, KbLeave, KbKey, KbModifiers, KbRepeatInfo
};

static void PointerEnter(void *data,
		      struct wl_pointer *wl_pointer,
		      uint32_t serial,
		      struct wl_surface *surface,
		      wl_fixed_t surface_x,
		      wl_fixed_t surface_y)
{
    OSWaylandData* wayLandData = NULL;
    int i = 0;
    for (i = 0; i < maxFreeListEntry; i++)
    {
        if (instancePointers[i].surface == surface)
        {
            wayLandData = &instancePointers[i];
            break;
        }
    }

    if (wayLandData == NULL)
    {
        return;
    }

    pointerActiveSurface = i;

    int x = wl_fixed_to_int(surface_x);
    int y = wl_fixed_to_int(surface_y);

    GenericWindowEventPacked* packed = GetWindowEventPacked(wayLandData->windowEventBuffer);
    packed->EventType = WINDOW_EVENT_TYPE_MOUSE_COORDINATES;
    packed->EventPacked = PACK_WINDOW_COORDINATES_EVENT(x, y);
    CommitWindowEventPacked(wayLandData->windowEventBuffer);

   // printf("surface pointer enter: surfaceId=%d, x=%d, y=%d, width=%d, height=%d\n", pointerActiveSurface, x, y, wayLandData->width, wayLandData->height);
}

static void PointerLeave(void *data,
		      struct wl_pointer *wl_pointer,
		      uint32_t serial,
		      struct wl_surface *surface)
{
    OSWaylandData* wayLandData = NULL;
    int i = 0;
    for (i = 0; i < maxFreeListEntry; i++)
    {
        if (instancePointers[i].surface == surface)
        {
            wayLandData = &instancePointers[i];
            break;
        }
    }

    if (wayLandData == NULL)
    {
        return;
    }

    pointerActiveSurface = -1;

   // printf("surface pointer exit: surfaceId=%d\n", i);
}

static void PointerMotion(void *data,
		       struct wl_pointer *wl_pointer,
		       uint32_t time,
		       wl_fixed_t surface_x,
		       wl_fixed_t surface_y)
{
    OSWaylandData* wayLandData = &instancePointers[pointerActiveSurface];
   
    int x = wl_fixed_to_int(surface_x);
    int y = wl_fixed_to_int(surface_y);

    GenericWindowEventPacked* packed = GetWindowEventPacked(wayLandData->windowEventBuffer);
    packed->EventType = WINDOW_EVENT_TYPE_MOUSE_COORDINATES;
    packed->EventPacked = PACK_WINDOW_COORDINATES_EVENT(x, y);
    CommitWindowEventPacked(wayLandData->windowEventBuffer);

   // printf("surface pointer move: surfaceId=%d, x=%d, y=%d, width=%d, height=%d\n", pointerActiveSurface, x, y, wayLandData->width, wayLandData->height);
}

static void PointerButton(void *data,
		       struct wl_pointer *wl_pointer,
		       uint32_t serial,
		       uint32_t time,
		       uint32_t button,
		       uint32_t state)
{
    OSWaylandData* wayLandData = &instancePointers[pointerActiveSurface];

    if (button == BTN_LEFT)
    {
        GenericWindowEventPacked* packed = GetWindowEventPacked(wayLandData->windowEventBuffer);
        
        packed->EventType = WINDOW_EVENT_TYPE_MOUSE_LEFT_BUTTON;

        if (state == WL_POINTER_BUTTON_STATE_RELEASED)
        {
            packed->EventPacked = 0;
        }
        else if (state == WL_POINTER_BUTTON_STATE_PRESSED)
        {
            packed->EventPacked = 1;
        }
        
        CommitWindowEventPacked(wayLandData->windowEventBuffer);
    }

  //  printf("surface pointer button: surfaceId=%d, button=%d, state=%d\n", pointerActiveSurface, button, state);
}

	
static void PointerAxis(void *data,
		     struct wl_pointer *wl_pointer,
		     uint32_t time,
		     uint32_t axis,
		     wl_fixed_t value)
{

}
	
static void PointerFrame(void *data,
		      struct wl_pointer *wl_pointer)
{

}
	
static void PointerAxisSource(void *data,
			    struct wl_pointer *wl_pointer,
			    uint32_t axis_source)
{

}

static void PointerAxisStop(void *data,
			  struct wl_pointer *wl_pointer,
			  uint32_t time,
			  uint32_t axis)
{

}

static void PointerAxisDiscrete(void *data,
			      struct wl_pointer *wl_pointer,
			      uint32_t axis,
			      int32_t discrete)
{

}

static void PointerAxisValue120(void *data,
			      struct wl_pointer *wl_pointer,
			      uint32_t axis,
			      int32_t value120)
{

}
	
static void PointerAxisRelativeDirection(void *data,
					struct wl_pointer *wl_pointer,
					uint32_t axis,
					uint32_t direction)
{

}

static const struct wl_pointer_listener pointer_listener =
{
    PointerEnter, PointerLeave, PointerMotion, 
    PointerButton, PointerAxis, PointerFrame, 
    PointerAxisSource, PointerAxisStop, PointerAxisDiscrete, 
    PointerAxisValue120, PointerAxisRelativeDirection
};

static void SeatCapabilities(void *data, struct wl_seat *seat, uint32_t caps)
{
    if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !keyboard)
    {
        keyboard = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(keyboard, &keyboard_listener, NULL);
    }
    else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && keyboard)
    {
        wl_keyboard_release(keyboard);
        keyboard = NULL;
    }

    if ((caps & WL_SEAT_CAPABILITY_POINTER) && !pointer)
    {
        pointer = wl_seat_get_pointer(seat);
        wl_pointer_add_listener(pointer, &pointer_listener, NULL);
    }
    else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && pointer)
    {
        wl_pointer_release(pointer);
        pointer = NULL;
    }
}

static void SeatName(void *data, struct wl_seat *seat, const char *name) 
{

}

static const struct wl_seat_listener seat_listener = 
{ 
    SeatCapabilities, SeatName 
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

    if (!compositor || !xdg_wm_base || !decoration_manager || !seat)
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

    wl_seat_add_listener(seat, &seat_listener, NULL);

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
        if (xkb_ctx)
        {
            xkb_context_unref(xkb_ctx);
        }

        xkb_ctx = NULL;
        
        if (keymap)
        {
            xkb_keymap_unref(keymap);
        }

        keymap = NULL; 

        if (keystate)
        {
            xkb_state_unref(keystate);
        }

        keystate = NULL;

        pointerActiveSurface = -1;
        keyboardActiveSurface = -1;

        zxdg_decoration_manager_v1_destroy(decoration_manager);
        xdg_wm_base_destroy(xdg_wm_base);
#ifdef USE_BUFFER
        wl_shm_destroy(shm);
        shm = NULL;
#endif
        wl_seat_destroy(seat);
        wl_compositor_destroy(compositor);
        wl_display_disconnect(display);

        decoration_manager = NULL;
        xdg_wm_base = NULL;
        seat = NULL;
        compositor = NULL;
        display = NULL;
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

    memcpy(data->windowHeader, name, strnlen(name, WINDOW_HEADER_TEXT_MAX_LEN-1));

    wl_surface_set_user_data(surface, data);

    windowData->internalOSHandle = osIndex;

    return OS_WINDOW_SUCCESS;
}

int OSWindowPollEvents(OSWindow* window, GenericWindowInfo* info)
{
    int ret = OS_WINDOW_SUCCESS;

    wl_display_dispatch_pending(display);
    wl_display_flush(display);

    int count = PumpWindowEventsPacked(&window->eventBuffer, info);

    return ret;
}

int OSWindowSetText(OSWindow* window, const char* text)
{
    OSWaylandData* data = &instancePointers[window->internalOSHandle];

    int count = strnlen(text, WINDOW_HEADER_TEXT_MAX_LEN-1);

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

    xdg_toplevel_add_listener(xdg_toplevel, &xdg_toplevel_listener, data);

    xdg_toplevel_set_title(data->xdg_toplevel, data->windowHeader);
    
    wl_surface_commit(data->surface); 
    wl_display_flush(display);
    
    while (!data->surfaceConfigured) 
    {
        wl_display_dispatch(display);    
    }

#ifdef USE_BUFFER
    if (data->buffer)
    {
        wl_surface_attach(data->surface, data->buffer, 0, 0);
        wl_surface_damage_buffer(data->surface, 0, 0, data->width, data->height);
        wl_surface_commit(data->surface);
        wl_display_flush(display);
    }
#endif

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

int OSWindowGetInternalData(OSWindow* window, void* internalDataStruct)
{
    int windowIndex = window->internalOSHandle;

    if (windowIndex < 0 || windowIndex >= maxFreeListEntry)
    {
        return OS_WINDOW_HANDLE_OUT_OF_BOUNDS;
    }

    OSWindowInternalData* data = (OSWindowInternalData*)internalDataStruct;

    data->display = display;
    data->surface = instancePointers[windowIndex].surface;

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