#pragma once

#include <cstdint>

#define IDLE_POLL_MODE 0
#define ACTIVE_POLL_MODE 1

namespace Peach {

/**
 * @brief Platform-agnostic lifecycle contract for a windowing/display
 * backend (Wayland today; X11/macOS/Windows are meant to be able to
 * implement this same interface later).
 *
 * Only what generalizes across platforms lives here: connect/pump/teardown
 * and the current drawable size. Anything specific to one platform's
 * windowing protocol (e.g. Wayland's compositor/output/layer-shell
 * objects) belongs on the concrete backend class, not here - forcing
 * platform-specific concepts into this interface would defeat the point
 * of having it.
 */
class WindowSystemBackend {
  public:
    virtual ~WindowSystemBackend() = default;

    /**
     * @brief Connects to the platform's display server and prepares the
     * wallpaper surface.
     * @return true when the backend is ready to render into.
     */
    virtual bool Initialize() = 0;

    /**
     * @brief Pumps platform events using the requested scheduling mode.
     * @param poll_mode ACTIVE_POLL_MODE checks for events without waiting;
     * IDLE_POLL_MODE may block until a platform event arrives.
     * @return false if the connection is closed or unusable.
     */
    virtual bool ProcessEvents(uint32_t poll_mode) = 0;

    /** @brief Releases all resources owned by the backend. */
    virtual void Shutdown() = 0;

    /** @brief Returns whether initialization completed successfully. */
    virtual bool IsInitialized() const noexcept = 0;

    /** @brief Returns the current surface width in pixels. */
    virtual uint32_t GetSurfaceWidth() const = 0;

    /** @brief Returns the current surface height in pixels. */
    virtual uint32_t GetSurfaceHeight() const = 0;

    /**
     * @brief Returns the platform-specific display handle (e.g. a
     * wl_display*), opaque to callers, for a Renderer to bind to.
     */
    virtual void *GetNativeDisplayHandle() const = 0;

    /**
     * @brief Returns the platform-specific window handle (e.g. a
     * wl_surface*), opaque to callers, for a Renderer to bind to.
     */
    virtual void *GetNativeWindowHandle() const = 0;
};

} // namespace Peach
