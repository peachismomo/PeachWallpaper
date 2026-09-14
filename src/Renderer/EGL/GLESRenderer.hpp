#pragma once

#include "../Renderer.hpp"

#include <wayland-egl.h>

#include <EGL/egl.h>

namespace Peach {
class GLESRenderer : public Renderer {
  public:
    /** @brief Creates an uninitialized OpenGL ES renderer. */
    GLESRenderer();

    /** @brief Releases the EGL and OpenGL ES resources. */
    ~GLESRenderer() override;

    /**
     * @brief Creates an EGL context and binds it to a Wayland surface.
     * @param native_display A `wl_display*` passed through the platform-
     * agnostic renderer interface.
     * @param native_window A `wl_surface*` passed through the platform-
     * agnostic renderer interface.
     * @param width Initial surface width in pixels.
     * @param height Initial surface height in pixels.
     * @return true when the EGL context and surface are ready.
     */
    bool Initialize(void *native_display, void *native_window, uint32_t width,
                    uint32_t height) override;

    /** @brief Resizes the Wayland EGL window to the requested dimensions. */
    bool Resize(uint32_t width, uint32_t height) override;

    /** @brief Clears the frame and draws the supplied renderable. */
    void Render(std::shared_ptr<Renderable> renderable) override;

    /** @brief Presents the current EGL back buffer to the compositor. */
    bool SwapBuffers() override;

    /** @brief Enables or disables the EGL swap interval for future frames. */
    void SetVSync(bool enabled) override;

    /** @brief Releases all EGL and OpenGL ES resources. */
    void Shutdown() override;

    /** @brief Returns whether EGL and the OpenGL ES context are initialized. */
    bool IsInitialized() const noexcept override;

  private:
    bool m_vsync{true};

    EGLint m_major_ver{};
    EGLint m_minor_ver{};
    EGLDisplay m_egl_display{nullptr};
    EGLConfig m_config{nullptr};
    EGLContext m_egl_ctx{nullptr};
    EGLSurface m_egl_surface{nullptr};

    wl_egl_window *m_egl_window{nullptr};

    bool m_initialized{false};

    uint32_t m_width{};
    uint32_t m_height{};
};
} // namespace Peach
