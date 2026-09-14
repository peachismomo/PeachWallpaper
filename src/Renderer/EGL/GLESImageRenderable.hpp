#pragma once

#include "../Renderable.hpp"

namespace Peach {
class GLESImageRenderable : public Renderable {
  public:
    /** @brief Creates an image renderable from its configuration path. */
    explicit GLESImageRenderable(const std::string &config);

    /** @brief Releases image and GPU resources. */
    ~GLESImageRenderable() override { Destroy(); }

    GLESImageRenderable() = delete;
    GLESImageRenderable(const GLESImageRenderable &) = delete;
    GLESImageRenderable &operator=(const GLESImageRenderable &) = delete;
    GLESImageRenderable(GLESImageRenderable &&) = delete;
    GLESImageRenderable &operator=(GLESImageRenderable &&) = delete;

    /** @brief Advances image-animation state, if the format supports it. */
    void Update(float time) override;

    /** @brief Draws the configured image. */
    void Draw() override;

    /** @brief Releases image and GPU resources. */
    void Destroy() override;

  private:
};
} // namespace Peach
