#include "PeachWallper.hpp"

#include "Backend/Wayland/WaylandBackend.hpp"
#include "Renderer/EGL/GLESRenderer.hpp"

#include "Renderer/EGL/GLESShaderRenderable.hpp"
#include "ur_log/ur-log.h"

#include <atomic>
#include <filesystem>
#include <glaze/glaze.hpp>

#include <chrono>
#include <exception>
#include <glaze/json/generic.hpp>
#include <glaze/json/read.hpp>
#include <glaze/json/write.hpp>
#include <memory>

namespace Peach {
PeachWallpaper::PeachWallpaper()
    : m_backend(std::make_shared<WaylandBackend>()),
      m_renderer(std::make_shared<GLESRenderer>()) {}

void PeachWallpaper::Start() {
    if (!m_socket.Create() || !m_socket.Bind() || !m_socket.Listen()) {
        UR_CRITICAL("Failed to initialize socket");
        std::terminate();
    }

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

    if (!PollSocket()) {
        UR_CRITICAL("Failed to start socket polling");
        std::terminate();
    }
}

void PeachWallpaper::Update() {
    auto last_time = std::chrono::steady_clock::now();

    while (m_running) {
        DrainCommands();
        if (!m_running)
            break;

        auto now = std::chrono::steady_clock::now();
        float delta_time =
            std::chrono::duration<float>(now - last_time).count();
        last_time = now;

        if (m_renderable) {
            m_renderable->Update(delta_time);
            m_renderer->Render(m_renderable);
        }

        if (!m_renderer->SwapBuffers()) {
            m_running = false;
            break;
        }

        if (!PollEvents()) {
            m_running = false;
            break;
        }
    }
}

void PeachWallpaper::Shutdown() {
    m_running.store(false);
    m_polling.store(false);

    if (m_socket_poll_thread.joinable())
        m_socket_poll_thread.join();

    m_socket.Shutdown();
    // Renderables own GL objects, so destroy them while the EGL context is
    // still current and before the renderer tears that context down.
    m_renderable.reset();
    m_renderer->Shutdown();
    m_backend->Shutdown();
}

bool PeachWallpaper::LoadConfig(const std::string &config) {
    std::string buffer;
    if (!std::filesystem::exists(config)) {
        glz::write_file_json(m_config, config, buffer);
    } else {
        auto ec = glz::read_file_json(m_config, config, buffer);
        if (ec) {
            UR_ERROR("Failed to load app config: {}",
                     glz::format_error(ec, std::string{}));
            return false;
        }
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

bool PeachWallpaper::PollSocket() {
    if (m_socket_poll_thread.joinable())
        return false;

    m_polling.store(true);

    m_socket_poll_thread = std::thread([this]() {
        while (m_polling.load()) {
            if (!m_socket.Poll()) {
                m_running.store(false);
                break;
            }

            while (auto cmd = m_socket.TakeCommand()) {
                std::lock_guard lock(m_command_mutex);
                m_commands.push_back(*cmd);
            }
        }

        m_polling.store(false);
    });

    return true;
}

void PeachWallpaper::DrainCommands() {
    std::deque<uint8_t> commands;
    {
        std::lock_guard lock(m_command_mutex);
        commands.swap(m_commands);
    }

    for (const uint8_t command : commands)
        HandleCommand(command);
}

void PeachWallpaper::HandleCommand(uint8_t code) {
    switch (code) {
    case 1:
        m_running = false;
        break;
    default:
        break;
    }
}

} // namespace Peach
