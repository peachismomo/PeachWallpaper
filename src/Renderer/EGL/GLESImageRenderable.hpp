#pragma once

#include "../Renderable.hpp"

namespace Peach {
class GLESImageRenderable : public Renderable {
  public:
    explicit GLESImageRenderable(const std::string &config);
    ~GLESImageRenderable() override { Destroy(); }

    GLESImageRenderable() = delete;
    GLESImageRenderable(const GLESImageRenderable &) = delete;
    GLESImageRenderable &operator=(const GLESImageRenderable &) = delete;
    GLESImageRenderable(GLESImageRenderable &&) = delete;
    GLESImageRenderable &operator=(GLESImageRenderable &&) = delete;

    void Update(float time) override;
    void Draw() override;
    void Destroy() override;

  private:
};
} // namespace Peach
