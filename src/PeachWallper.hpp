#pragma once

#include "Backend/WindowSystemBackend.hpp"
#include "Renderer/Renderable.hpp"
#include "Renderer/Renderer.hpp"
#include <memory>

namespace Peach {
class PeachWallpaper {
  public:
    /** @brief Creates an unstarted wallpaper runtime. */
    PeachWallpaper();

    /** @brief Shuts down the runtime if it is still running. */
    ~PeachWallpaper() { Shutdown(); }

    /**
     * @brief Loads the wallpaper runtime configuration and creates its
     * renderable.
     * @param config Path to the application configuration file.
     * @return true when the configuration and requested renderable load.
     */
    bool LoadConfig(const std::string &config);

    /** @brief Initializes the window-system backend and renderer. */
    void Start();

    /**
     * @brief Runs the main update, event-processing, and presentation loop.
     *
     * The loop continues until the runtime stops, event processing fails, or
     * presenting a frame fails.
     */
    void Update();

    /** @brief Releases the backend and renderer resources. */
    void Shutdown();

  private:
    /** @brief Processes backend events using the current poll mode. */
    bool PollEvents();

    std::shared_ptr<WindowSystemBackend> m_backend;
    std::shared_ptr<Renderer> m_renderer;
    std::shared_ptr<Renderable> m_renderable;

    struct Config {
        enum RenderableType { IMAGE, VIDEO, SHADER, PARTICLE, Scene3D };
        RenderableType type;
        std::string wallpaper_file{};
        bool vsync{};
    };
    bool m_running{false};
    Config m_config{};
    uint32_t m_poll_mode{ACTIVE_POLL_MODE};
};
} // namespace Peach
