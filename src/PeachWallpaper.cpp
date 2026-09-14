#include "PeachWallper.hpp"

#include "Backend/Wayland/WaylandBackend.hpp"
#include "Renderer/EGL/GLESRenderer.hpp"

#include "Renderer/EGL/GLESShaderRenderable.hpp"
#include "ur_log/ur-log.h"

#include <glaze/glaze.hpp>

#include <chrono>
#include <exception>
#include <glaze/json/generic.hpp>
#include <glaze/json/read.hpp>
#include <memory>

namespace Peach {
PeachWallpaper::PeachWallpaper()
    : m_backend(std::make_shared<WaylandBackend>()),
      m_renderer(std::make_shared<GLESRenderer>()) {}

void PeachWallpaper::Start() {
    bool init = m_backend->Initialize();
    if (!init) {
        UR_CRITICAL("Failed to initialize backend");
        std::terminate();
    }

    init = m_renderer->Initialize(
        m_backend->GetNativeDisplayHandle(), m_backend->GetNativeWindowHandle(),
        m_backend->GetSurfaceWidth(), m_backend->GetSurfaceHeight());

    if (!init) {
        UR_CRITICAL("Failed to initialize renderer");
        std::terminate();
    }

    m_running = true;
}

void PeachWallpaper::Update() {
    auto last_time = std::chrono::steady_clock::now();

    while (m_running) {
        auto now = std::chrono::steady_clock::now();
        float delta_time =
            std::chrono::duration<float>(now - last_time).count();
        last_time = now;

        if (m_renderable) {
            m_renderable->Update(delta_time);
            m_renderer->Render(m_renderable);
        }

        if (!m_renderer->SwapBuffers()) {
            break;
        }

        if (!PollEvents())
            break;
    }
}

void PeachWallpaper::Shutdown() {
    if (!m_running)
        return;

    m_backend->Shutdown();
    m_renderer->Shutdown();

    m_running = false;
}

bool PeachWallpaper::LoadConfig(const std::string &config) {
    std::string buffer;
    auto ec = glz::read_file_json(m_config, config, buffer);
    if (ec) {
        UR_ERROR("Failed to load app config: {}",
                 glz::format_error(ec, std::string{}));
        return false;
    }

    m_renderer->SetVSync(m_config.vsync);

    switch (m_config.type) {
    case Config::IMAGE:
        UR_ERROR("Not implemented");
        break;
    case Config::VIDEO:
        UR_ERROR("Not implemented");
        break;
    case Config::SHADER:
        try {
            m_renderable =
                std::make_shared<GLESShaderRenderable>(m_config.wallpaper_file);
        } catch (const std::exception &e) {
            UR_ERROR("Failed to create shader renderable: {}", e.what());
            return false;
        }
        break;
    case Config::PARTICLE:
        UR_ERROR("Not implemented");
        break;
    case Config::Scene3D:
        UR_ERROR("Not implemented");
        break;
    default:
        break;
    }

    if (!m_renderable)
        return false;

    return true;
}

bool PeachWallpaper::PollEvents() {
    if (!m_backend->ProcessEvents(m_poll_mode))
        return false;

    return true;
}

} // namespace Peach
