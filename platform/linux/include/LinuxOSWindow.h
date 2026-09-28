#pragma once

#if defined(WINDOW_USE_WAYLAND)

#include <wayland-client.h>
#include <xdg-shell-client-protocol.h>
#include <xdg-decoration-client-protocol.h>

struct OSWindowInternalData
{
    struct wl_display* display;
    struct wl_surface* surface;
};

#elif defined(WINDOW_USE_X11)

struct OSWindowInternalData
{
    int placeholder;
};

#endif