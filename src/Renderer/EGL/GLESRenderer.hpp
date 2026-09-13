#pragma once

#include "../Renderer.hpp"

#include <wayland-egl.h>

#include <EGL/egl.h>

namespace Peach {
class GLESRenderer : public Renderer {
  public:
    GLESRenderer();
    ~GLESRenderer() override;
    bool Initialize(void *native_display, void *native_window, uint32_t width,
                    uint32_t height) override;
    bool Resize(uint32_t width, uint32_t height) override;
    void Render(std::shared_ptr<Renderable> renderable) override;
    bool SwapBuffers() override;
    void SetVSync(bool enabled) override;
    void Shutdown() override;
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