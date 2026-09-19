#include "GLESRenderer.hpp"

#include "GLES.hpp"
#include "ur_log/ur-log.h"

namespace Peach {
GLESRenderer::GLESRenderer() {}

GLESRenderer::~GLESRenderer() { Shutdown(); }

bool GLESRenderer::Initialize(void *native_display, void *native_window,
                              uint32_t width, uint32_t height) {
    m_egl_display = eglGetDisplay(static_cast<wl_display *>(native_display));

    if (!m_egl_display) {
        UR_ERROR("Failed to get EGL display");
        return false;
    }

    EGLBoolean ret = eglInitialize(m_egl_display, &m_major_ver, &m_minor_ver);
    if (ret != EGL_TRUE) {
        UR_ERROR("Failed to initialize EGL");
        Shutdown();
        return false;
    }
    m_egl_initialized = true;

    ret = eglBindAPI(EGL_OPENGL_ES_API);
    if (ret != EGL_TRUE) {
        UR_ERROR("Failed to bind EGL_OPENGL_ES_API");
        Shutdown();
        return false;
    }
    static EGLint s_config_attribs[] = {EGL_SURFACE_TYPE,
                                        EGL_WINDOW_BIT,
                                        EGL_RENDERABLE_TYPE,
                                        EGL_OPENGL_ES2_BIT,
                                        EGL_RED_SIZE,
                                        8,
                                        EGL_GREEN_SIZE,
                                        8,
                                        EGL_BLUE_SIZE,
                                        8,
                                        EGL_ALPHA_SIZE,
                                        8,
                                        EGL_NONE};
    EGLint num_config;

    ret = eglChooseConfig(m_egl_display, s_config_attribs, &m_config, 1,
                          &num_config);
    if (ret != EGL_TRUE || num_config == 0) {
        UR_ERROR("Failed to choose EGL config");
        Shutdown();
        return false;
    }
    m_egl_window = wl_egl_window_create(
        static_cast<wl_surface *>(native_window), width, height);

    if (!m_egl_window) {
        UR_ERROR("Failed to create EGL window");
        Shutdown();
        return false;
    }
    m_egl_surface =
        eglCreateWindowSurface(m_egl_display, m_config, m_egl_window, nullptr);

    if (!m_egl_surface) {
        UR_ERROR("Failed to create EGL surface");
        Shutdown();
        return false;
    }

    static const EGLint s_context_attribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2,
                                               EGL_NONE};

    m_egl_ctx = eglCreateContext(m_egl_display, m_config, EGL_NO_CONTEXT,
                                 s_context_attribs);

    if (!m_egl_ctx) {
        UR_ERROR("Failed to create EGL context");
        Shutdown();
        return false;
    }

    ret =
        eglMakeCurrent(m_egl_display, m_egl_surface, m_egl_surface, m_egl_ctx);
    if (ret != EGL_TRUE) {
        UR_ERROR("Failed to make EGL ctx current");
        Shutdown();
        return false;
    }

    m_width = width;
    m_height = height;

    m_initialized = true;
    return true;
}

bool GLESRenderer::Resize(uint32_t width, uint32_t height) {
    if (!m_egl_window) {
        UR_ERROR("Cannot resize before the renderer is initialized");
        return false;
    }

    m_width = width;
    m_height = height;
    wl_egl_window_resize(m_egl_window, m_width, m_height, 0, 0);
    return true;
}

void GLESRenderer::Render(std::shared_ptr<Renderable> renderable) {
    GL_CALL(glViewport(0, 0, m_width, m_height));
    GL_CALL(glClear(GL_COLOR_BUFFER_BIT));

    renderable->Draw();
}

bool GLESRenderer::SwapBuffers() {
    if (eglSwapInterval(m_egl_display, m_vsync ? 1 : 0) != EGL_TRUE) {
        UR_WARN("Failed to set EGL swap interval");
    }

    EGLBoolean ret = eglSwapBuffers(m_egl_display, m_egl_surface);
    if (ret != EGL_TRUE) {
        UR_ERROR("Failed to swap EGL buffers");
        return false;
    }
    return true;
}

void GLESRenderer::SetVSync(bool enabled) { m_vsync = enabled; }

void GLESRenderer::Shutdown() {
    if (m_egl_display && m_egl_initialized) {
        if (eglMakeCurrent(m_egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE,
                           EGL_NO_CONTEXT) != EGL_TRUE) {
            UR_ERROR("Failed to unbind EGL display");
        }
    }

    if (m_egl_ctx && m_egl_display && m_egl_initialized) {
        eglDestroyContext(m_egl_display, m_egl_ctx);
    }
    m_egl_ctx = nullptr;

    if (m_egl_surface && m_egl_display && m_egl_initialized) {
        eglDestroySurface(m_egl_display, m_egl_surface);
    }
    m_egl_surface = nullptr;

    if (m_egl_display && m_egl_initialized) {
        if (eglTerminate(m_egl_display) != EGL_TRUE) {
            UR_ERROR("Failed to terminate EGL");
        }
    }
    m_egl_display = nullptr;
    m_egl_initialized = false;

    if (m_egl_window) {
        wl_egl_window_destroy(m_egl_window);
        m_egl_window = nullptr;
    }

    m_initialized = false;
}

bool GLESRenderer::IsInitialized() const noexcept { return m_initialized; }

} // namespace Peach
