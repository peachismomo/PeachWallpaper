#pragma once

#include <string>

namespace Peach {
class Renderable {
  public:
    explicit Renderable(const std::string &config) : m_config(config) {}
    virtual ~Renderable() = default;

    Renderable() = delete;
    Renderable(const Renderable &) = delete;
    Renderable &operator=(const Renderable &) = delete;
    Renderable(Renderable &&) = delete;
    Renderable &operator=(Renderable &&) = delete;

    virtual void Update(float time) = 0;
    virtual void Draw() = 0;
    virtual void Destroy() = 0;

  protected:
    std::string m_config;
};
} // namespace Peach
