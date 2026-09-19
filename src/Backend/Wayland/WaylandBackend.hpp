#pragma once

#include "../WindowSystemBackend.hpp"
#include "wlr-layer-shell-client-protocol.h"
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>

namespace Peach {

/**
 * @brief Owns the Wayland connection and wallpaper layer surface.
 *
 * This is the platform boundary for the wallpaper runtime. It owns Wayland
 * protocol objects and event dispatch, but not EGL/OpenGL rendering or
 * wallpaper playback.
 *
 * Every `wl_*`/`zwlr_*` pointer this class holds is a *proxy*, not the real
 * thing. Wayland is a client/server protocol: the compositor process (Sway,
 * KWin, Mutter, ...) owns the actual output, input, and surface state on the
 * other end of a Unix socket. A proxy is just a local C struct identifying a
 * numbered object on that connection - calling a `wl_foo_*()` function on it
 * serializes a request to the server, and the server replies with events
 * that arrive asynchronously and get delivered to the listener callbacks
 * registered below. Nothing here draws pixels; it only negotiates *where*
 * and *how big* the surface this backend owns should be.
 */
class WaylandBackend : public WindowSystemBackend {
  public:
    /** @brief Creates an uninitialized backend. */
    WaylandBackend();

    /** @brief Releases resources owned by the backend. */
    ~WaylandBackend() override;

    WaylandBackend(const WaylandBackend &) = delete;
    WaylandBackend &operator=(const WaylandBackend &) = delete;
    WaylandBackend(WaylandBackend &&) = delete;
    WaylandBackend &operator=(WaylandBackend &&) = delete;

    /**
     * @brief Connects to Wayland and prepares the wallpaper surface.
     * @return true when the required Wayland objects are ready.
     */
    bool Initialize() override;

    /**
     * @brief Pumps the Wayland connection using the requested poll mode.
     * @param poll_mode ACTIVE_POLL_MODE checks for compositor events without
     * waiting; IDLE_POLL_MODE blocks until a Wayland event arrives.
     * @return false if the connection is closed or unusable.
     */
    bool ProcessEvents(uint32_t poll_mode) override;

    /** @brief Releases all Wayland resources owned by the backend. */
    void Shutdown() override;

    /** @brief Returns whether initialization completed successfully. */
    bool IsInitialized() const noexcept override;

    /** @brief Stores the bound Wayland compositor. */
    void SetCompositor(wl_compositor *compositor);

    /** @brief Returns the bound Wayland compositor. */
    wl_compositor *GetCompositor();

    /** @brief Stores the selected monitor output. */
    void SetOutput(wl_output *output);

    /** @brief Returns the selected monitor output. */
    wl_output *GetOutput();

    /** @brief Stores the bound layer-shell factory global. */
    void SetLayerShell(zwlr_layer_shell_v1 *layer_shell);

    /** @brief Returns the bound layer-shell factory global. */
    zwlr_layer_shell_v1 *GetLayerShell();

    /** @brief Stores the plain wl_surface backing the wallpaper. */
    void SetSurface(wl_surface *surface);

    /** @brief Returns the plain wl_surface backing the wallpaper. */
    wl_surface *GetSurface();

    /** @brief Stores the layer-shell role wrapping the wallpaper surface. */
    void SetLayerSurface(zwlr_layer_surface_v1 *layer_surface);

    /** @brief Returns the layer-shell role wrapping the wallpaper surface. */
    zwlr_layer_surface_v1 *GetLayerSurface();

    void SetSeat(wl_seat* seat);
    wl_keyboard* GetSeat();

    /** @brief Stores the Wayland display connection. */
    void SetDisplay(wl_display *display);

    /** @brief Returns the Wayland display connection. */
    wl_display *GetDisplay();

    /** @brief Stores the registry used to discover compositor globals. */
    void SetRegistry(wl_registry *registry);

    /** @brief Returns the registry used to discover compositor globals. */
    wl_registry *GetRegistry();

    /** @brief Stores the surface width negotiated via the last configure. */
    void SetSurfaceWidth(uint32_t width);

    /** @brief Returns the surface width negotiated via the last configure. */
    uint32_t GetSurfaceWidth() const override;

    /** @brief Stores the surface height negotiated via the last configure. */
    void SetSurfaceHeight(uint32_t height);

    /** @brief Returns the surface height negotiated via the last configure. */
    uint32_t GetSurfaceHeight() const override;

    /** @brief Returns the Wayland display connection as an opaque handle. */
    void *GetNativeDisplayHandle() const override;

    /** @brief Returns the plain wl_surface as an opaque handle. */
    void *GetNativeWindowHandle() const override;

  private:
    /**
     * @brief Handles registry announcements and binds supported globals.
     * @param data The WaylandBackend receiving the announcement.
     * @param wl_registry Registry that emitted the event.
     * @param name Compositor-assigned global name.
     * @param interface Advertised Wayland interface name.
     * @param version Highest version advertised by the compositor.
     */
    static void global_listener(void *data, struct wl_registry *wl_registry,
                                uint32_t name, const char *interface,
                                uint32_t version);

    /**
     * @brief Handles the registry's global_remove event: a previously
     * advertised global has disappeared (most relevantly, `m_output` being
     * unplugged). This is where the backend learns an output disappeared,
     * and where notifying the owner of that would be invoked.
     */
    static void global_listener_remove(void *data,
                                       struct wl_registry *wl_registry,
                                       uint32_t name);

    /**
     * @brief Handles the layer surface's configure event: the compositor
     * is telling this backend what size the surface should be. This is
     * where the backend learns the size changed, and where notifying the
     * owner of a resize would be invoked.
     */
    static void layer_surface_configure(void *data,
                                        zwlr_layer_surface_v1 *surface,
                                        uint32_t serial, uint32_t width,
                                        uint32_t height);

    /**
     * @brief Handles the layer surface's closed event: the compositor has
     * revoked this surface (e.g. its output was removed). This is where
     * the backend learns the surface was revoked, and where notifying the
     * owner of that would be invoked.
     */
    static void layer_surface_closed(void *data,
                                     zwlr_layer_surface_v1 *surface);

    /** @brief Checks whether the required Wayland globals were bound. */
    bool HasRequiredGlobals() const noexcept;

    /** @brief Creates and configures the background layer surface. */
    bool CreateLayerSurface();

    /** @brief Destroys all Wayland protocol objects. */
    void DestroyWaylandObjects() noexcept;

    /**
     * @brief The client-compositor connection (a Unix socket under the
     * hood). This is the root of everything else in this class: every other
     * proxy is a request/reply conversation carried over this connection,
     * and it's what gets dispatched/roundtripped to pump incoming events.
     * Obtained once from wl_display_connect() and held until Shutdown().
     */
    wl_display *m_display{nullptr};

    /**
     * @brief The "directory" object used to discover what the compositor
     * supports. The core Wayland protocol has no "get me the compositor"
     * call; instead the server advertises a list of named globals (a
     * numeric name, an interface string like "wl_compositor", and a
     * version) over this object, one at a time, as asynchronous `global`
     * events. global_listener() is this backend's handler for those events;
     * it's how compositor/output/layer-shell below get bound in the first
     * place. Obtained once from wl_display_get_registry(display).
     */
    wl_registry *m_registry{nullptr};

    /**
     * @brief The bound wl_compositor global - a *factory*, not a window.
     * Its only real job is minting wl_surface objects (blank, contentless
     * rectangles) via wl_compositor_create_surface(). Binding it doesn't
     * give the backend anything visible yet; it just grants the ability to
     * later create the canvas the wallpaper will render into.
     */
    wl_compositor *m_compositor{nullptr};

    /**
     * @brief The chosen monitor. One wl_output proxy represents one
     * physical/logical display known to the compositor, and (like the
     * registry) reports its own geometry/mode/scale asynchronously via its
     * own events. Needed both to pick which screen the wallpaper surface
     * should live on and to read output scale for correct HiDPI rendering.
     */
    wl_output *m_output{nullptr};

    /**
     * @brief The bound zwlr_layer_shell_v1 global - a *factory*, like
     * `m_compositor`, not a surface itself. Its only real job is one
     * request, get_layer_surface(), which wraps a plain wl_surface with the
     * layer-shell role. Bound directly from the registry the same way
     * compositor/output are.
     */
    zwlr_layer_shell_v1 *m_layer_shell{nullptr};

    /**
     * @brief The plain, contentless canvas this backend renders into.
     * Created via wl_compositor_create_surface(m_compositor). On its own it
     * has no position or stacking behavior - it only becomes a background
     * surface once wrapped by `m_layer_surface` below.
     */
    wl_surface *m_surface{nullptr};

    wl_keyboard* m_keyboard{nullptr};
    wl_seat* m_seat{nullptr};

    /**
     * @brief The layer-shell role wrapping `m_surface`, turning it into a
     * compositor-managed background/panel/overlay surface (as opposed to a
     * normal decorated window, which would use xdg_shell instead). Created
     * via m_layer_shell->get_layer_surface(m_surface, m_output, ...). Its
     * configure/closed events (see layer_surface_configure/
     * layer_surface_closed) are how the compositor tells this backend what
     * size the surface should be and when it's being revoked.
     */
    zwlr_layer_surface_v1 *m_layer_surface{nullptr};

    /** @brief Whether Initialize() has completed successfully. Gates
     * ProcessEvents()/state queries so callers can't use a half-connected
     * backend. */
    bool m_initialized{false};

    /** @brief Surface width/height as last negotiated via the layer
     * surface's configure event (0 until the first configure arrives). */
    uint32_t m_surface_width{};
    uint32_t m_surface_height{};
};
} // namespace Peach
