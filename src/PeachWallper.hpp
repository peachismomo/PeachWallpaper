#pragma once

#include "Backend/Backend.hpp"
#include "Renderer/Renderable.hpp"
#include "Renderer/Renderer.hpp"
#include <memory>

namespace Peach {
class PeachWallpaper {
  public:
    PeachWallpaper();
    ~PeachWallpaper() { Shutdown(); }

    bool LoadConfig(const std::string &config);
    void Start();
    void Update();
    void Shutdown();

  private:
    struct Config {
        enum RenderableType { IMAGE, VIDEO, SHADER, PARTICLE, Scene3D };
        RenderableType type;
        std::string wallpaper_file{};
        bool vsync{};
    };

    std::shared_ptr<Backend> m_backend;
    std::shared_ptr<Renderer> m_renderer;
    std::shared_ptr<Renderable> m_renderable;

    bool m_running{false};

    Config m_config{};
};
} // namespace Peach