#pragma once

#include "Renderable.hpp"
#include <cstdint>
#include <memory>

namespace Peach {

/**
 * @brief Platform-agnostic contract for a rendering backend (EGL/GLES
 * first; Vulkan or other APIs are meant to be able to implement this same
 * interface later).
 *
 * Takes native display/window handles as opaque `void*` so this interface
 * never needs to know about any windowing protocol's types (e.g. Wayland's
 * wl_surface). The concrete implementation casts them back to whatever its
 * graphics API actually expects.
 */
class Renderer {
  public:
    /** @brief Releases resources owned by the concrete renderer. */
    virtual ~Renderer() = default;

    /**
     * @brief Creates the rendering context and binds it to the given
     * native window.
     * @param native_display Platform-specific display handle (e.g. a
     * wl_display*), opaque to this interface.
     * @param native_window Platform-specific window handle (e.g. a
     * wl_surface*), opaque to this interface.
     * @param width Initial drawable width in pixels.
     * @param height Initial drawable height in pixels.
     * @return true when the renderer is ready to draw.
     */
    virtual bool Initialize(void *native_display, void *native_window,
                            uint32_t width, uint32_t height) = 0;

    /**
     * @brief Resizes the drawable surface, e.g. after the windowing
     * backend renegotiates size.
     * @return true if the resize succeeded.
     */
    virtual bool Resize(uint32_t width, uint32_t height) = 0;

    /** @brief Issues draw calls for the current frame. */
    virtual void Render(std::shared_ptr<Renderable> renderable) = 0;

    /**
     * @brief Presents the rendered frame.
     * @return false if the swap failed (e.g. the surface became invalid).
     */
    virtual bool SwapBuffers() = 0;

    /**
     * @brief Sets whether presenting a frame should wait for the display's
     * refresh (vsync). Takes effect on the next SwapBuffers() call.
     */
    virtual void SetVSync(bool enabled) = 0;

    /** @brief Releases all resources owned by the renderer. */
    virtual void Shutdown() = 0;

    /** @brief Returns whether initialization completed successfully. */
    virtual bool IsInitialized() const noexcept = 0;
};

} // namespace Peach
