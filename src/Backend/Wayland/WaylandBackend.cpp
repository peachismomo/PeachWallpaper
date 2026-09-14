#include "WaylandBackend.hpp"

#include "ur_log/ur-log.h"

#include "wlr-layer-shell-client-protocol.h"
#include <sys/poll.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-util.h>

#include <cstddef>
#include <cstring>
#include <functional>
#include <poll.h>

#define POLL_TIMEOUT_MS 0

namespace Peach {
WaylandBackend::WaylandBackend() {}
WaylandBackend::~WaylandBackend() { Shutdown(); }

void WaylandBackend::global_listener(void *data,
                                     struct wl_registry *wl_registry,
                                     uint32_t name, const char *interface,
                                     uint32_t version) {
    WaylandBackend *backend = static_cast<WaylandBackend *>(data);

    std::function<void *(const wl_interface *)> bind =
        [&](const wl_interface *iface) {
            return wl_registry_bind(wl_registry, name, iface,
                                    std::min(version, 6u));
        };

    if (std::strcmp(interface, wl_compositor_interface.name) == 0) {
        backend->SetCompositor(
            static_cast<wl_compositor *>(bind(&wl_compositor_interface)));
    }

    if (std::strcmp(interface, wl_output_interface.name) == 0) {
        backend->SetOutput(
            static_cast<wl_output *>(bind(&wl_output_interface)));
    }

    if (std::strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
        backend->SetLayerShell(static_cast<zwlr_layer_shell_v1 *>(
            bind(&zwlr_layer_shell_v1_interface)));
    }
}

void WaylandBackend::global_listener_remove(void *data,
                                            struct wl_registry *wl_registry,
                                            uint32_t name) {}

bool WaylandBackend::Initialize() {
    m_display = wl_display_connect(NULL);

    if (!m_display) {
        UR_ERROR("Failed to connect to wayland display");
        return false;
    }

    m_registry = wl_display_get_registry(m_display);

    if (!m_registry) {
        UR_ERROR("Failed to get wayland registry");
        return false;
    }

    static wl_registry_listener listener{&global_listener,
                                         &global_listener_remove};

    wl_registry_add_listener(m_registry, &listener, this);

    int ret = wl_display_roundtrip(m_display);

    if (ret == -1) {
        UR_ERROR("Wayland display roundtrip failed");
        return false;
    }

    if (!HasRequiredGlobals())
        return false;

    if (!CreateLayerSurface())
        return false;

    if (wl_display_roundtrip(m_display) == -1) {
        UR_ERROR("Wayland display roundtrip failed");
        return false;
    }
    m_initialized = true;
    return true;
}

bool WaylandBackend::ProcessEvents(uint32_t poll_mode) {
    if (poll_mode == IDLE_POLL_MODE) {
        return wl_display_dispatch(m_display) != -1;
    }

    // wl_display_dispatch_pending() only processes events already sitting
    // in the local queue - it never reads the socket - so this follows
    // libwayland's prepare_read/poll/read_events pattern to check for new
    // data without blocking. This must stay non-blocking: it runs once per
    // rendered frame, and frame pacing is the render loop's job, not this
    // function's.
    while (wl_display_prepare_read(m_display) != 0)
        wl_display_dispatch_pending(m_display);

    if (wl_display_flush(m_display) == -1) {
        UR_WARN("Failed to flush wayland events");
        wl_display_cancel_read(m_display);
        return false;
    }

    pollfd pfd{wl_display_get_fd(m_display), POLLIN, 0};
    int ret = poll(&pfd, 1, POLL_TIMEOUT_MS);

    if (ret < 0) {
        UR_WARN("Failed to poll wayland display fd");
        wl_display_cancel_read(m_display);
        return false;
    }

    if (pfd.revents & POLLIN) {
        if (wl_display_read_events(m_display) == -1) {
            UR_WARN("Failed to read wayland events");
            return false;
        }
    } else {
        wl_display_cancel_read(m_display);
    }

    if (wl_display_dispatch_pending(m_display) == -1) {
        UR_WARN("Failed to dispatch wayland events");
        return false;
    }

    return true;
}

void WaylandBackend::Shutdown() {
    DestroyWaylandObjects();
    m_initialized = false;
}

bool WaylandBackend::IsInitialized() const noexcept { return m_initialized; }

void WaylandBackend::SetDisplay(wl_display *value) { m_display = value; }

wl_display *WaylandBackend::GetDisplay() { return m_display; }

void WaylandBackend::SetRegistry(wl_registry *value) { m_registry = value; }

wl_registry *WaylandBackend::GetRegistry() { return m_registry; }

void WaylandBackend::SetCompositor(wl_compositor *value) {
    m_compositor = value;
}

wl_compositor *WaylandBackend::GetCompositor() { return m_compositor; }

void WaylandBackend::SetOutput(wl_output *value) { m_output = value; }

wl_output *WaylandBackend::GetOutput() { return m_output; }

void WaylandBackend::SetLayerShell(zwlr_layer_shell_v1 *value) {
    m_layer_shell = value;
}

zwlr_layer_shell_v1 *WaylandBackend::GetLayerShell() { return m_layer_shell; }

void WaylandBackend::SetSurface(wl_surface *value) { m_surface = value; }

wl_surface *WaylandBackend::GetSurface() { return m_surface; }

void WaylandBackend::SetLayerSurface(zwlr_layer_surface_v1 *value) {
    m_layer_surface = value;
}

zwlr_layer_surface_v1 *WaylandBackend::GetLayerSurface() {
    return m_layer_surface;
}

void WaylandBackend::SetSurfaceWidth(uint32_t value) {
    m_surface_width = value;
}

uint32_t WaylandBackend::GetSurfaceWidth() const { return m_surface_width; }

void WaylandBackend::SetSurfaceHeight(uint32_t value) {
    m_surface_height = value;
}

uint32_t WaylandBackend::GetSurfaceHeight() const { return m_surface_height; }

void *WaylandBackend::GetNativeDisplayHandle() const { return m_display; }

void *WaylandBackend::GetNativeWindowHandle() const { return m_surface; }

void WaylandBackend::layer_surface_configure(
    void *data, zwlr_layer_surface_v1 *layer_surface, uint32_t serial,
    uint32_t width, uint32_t height) {
    WaylandBackend *backend = static_cast<WaylandBackend *>(data);
    zwlr_layer_surface_v1_ack_configure(layer_surface, serial);

    backend->SetSurfaceWidth(width);
    backend->SetSurfaceHeight(height);

    // any callbacks to resize ?
}

void WaylandBackend::layer_surface_closed(
    void *data, zwlr_layer_surface_v1 *layer_surface) {
    WaylandBackend *backend = static_cast<WaylandBackend *>(data);

    if (backend->GetLayerSurface() == layer_surface) {
        UR_WARN("Layer surface was closed");

        zwlr_layer_surface_v1_destroy(layer_surface);
        wl_surface_destroy(backend->GetSurface());
        backend->SetSurface(nullptr);
        backend->SetLayerSurface(nullptr);

        backend->m_initialized = false;
    }

    // callbacks, if any
}

bool WaylandBackend::HasRequiredGlobals() const noexcept {
    if (!m_display) {
        UR_ERROR("No wayland display found");
        return false;
    }

    if (!m_registry) {
        UR_ERROR("No wayland registry found");
        return false;
    }

    if (!m_compositor) {
        UR_ERROR("No wayland compositor found");
        return false;
    }

    if (!m_output) {
        UR_ERROR("No wayland output found");
        return false;
    }

    if (!m_layer_shell) {
        UR_ERROR("No wayland layer_shell found");
        return false;
    }
    return true;
}

bool WaylandBackend::CreateLayerSurface() {
    m_surface = wl_compositor_create_surface(m_compositor);

    if (!m_surface) {
        UR_ERROR("Failed to create wayland surface");
        return false;
    }

    m_layer_surface = zwlr_layer_shell_v1_get_layer_surface(
        m_layer_shell, m_surface, m_output,
        ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND, "peach_wallpaper");

    if (!m_layer_surface) {
        UR_ERROR("Failed to create wayland layer surface");
        return false;
    }

    static zwlr_layer_surface_v1_listener layer_surface_listener{
        &layer_surface_configure, &layer_surface_closed};

    int ret = zwlr_layer_surface_v1_add_listener(m_layer_surface,
                                                 &layer_surface_listener, this);

    if (ret != 0)
        UR_WARN("Layer surface already has listener");

    zwlr_layer_surface_v1_set_anchor(m_layer_surface,
                                     ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
                                         ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM |
                                         ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT |
                                         ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT);
    wl_surface_commit(m_surface);

    return true;
}

void WaylandBackend::DestroyWaylandObjects() noexcept {
    if (m_layer_surface) {
        zwlr_layer_surface_v1_destroy(m_layer_surface);
        m_layer_surface = nullptr;
    }

    if (m_surface) {
        wl_surface_destroy(m_surface);
        m_surface = nullptr;
    }

    if (m_layer_shell) {
        zwlr_layer_shell_v1_destroy(m_layer_shell);
        m_layer_shell = nullptr;
    }

    if (m_compositor) {
        wl_compositor_destroy(m_compositor);
        m_compositor = nullptr;
    }

    if (m_output) {
        wl_output_release(m_output);
        m_output = nullptr;
    }

    if (m_registry) {
        wl_registry_destroy(m_registry);
        m_registry = nullptr;
    }

    if (m_display) {
        wl_display_disconnect(m_display);
        m_display = nullptr;
    }
}
} // namespace Peach
