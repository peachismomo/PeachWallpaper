#pragma once

#include <string>

namespace Peach {
class Renderable {
  public:
    /** @brief Creates a renderable configured by the given asset path. */
    explicit Renderable(const std::string &config) : m_config(config) {}

    /** @brief Releases resources owned by the renderable. */
    virtual ~Renderable() = default;

    /** @brief Renderables require a configuration path at construction. */
    Renderable() = delete;
    Renderable(const Renderable &) = delete;
    Renderable &operator=(const Renderable &) = delete;
    Renderable(Renderable &&) = delete;
    Renderable &operator=(Renderable &&) = delete;

    /** @brief Advances content state by the elapsed time in seconds. */
    virtual void Update(float time) = 0;

    /** @brief Issues the draw calls for the current content state. */
    virtual void Draw() = 0;

    /** @brief Releases graphics and content resources owned by the object. */
    virtual void Destroy() = 0;

  protected:
    std::string m_config;
};
} // namespace Peach
