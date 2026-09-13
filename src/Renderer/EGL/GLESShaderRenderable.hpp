#pragma once

#include "../Renderable.hpp"
#include "GLESShader.hpp"

#include <string>

namespace Peach {
class GLESShaderRenderable : public Renderable {
  public:
    explicit GLESShaderRenderable(const std::string &config);
    ~GLESShaderRenderable() override { Destroy(); }

    GLESShaderRenderable() = delete;
    GLESShaderRenderable(const GLESShaderRenderable &) = delete;
    GLESShaderRenderable &operator=(const GLESShaderRenderable &) = delete;
    GLESShaderRenderable(GLESShaderRenderable &&) = delete;
    GLESShaderRenderable &operator=(GLESShaderRenderable &&) = delete;

    void Update(float time) override;
    void Draw() override;
    void Destroy() override;

  private:
    struct Vertex {
        glm::vec2 pos;
        glm::vec2 uv;
    };

    float m_time{};

    GLESShader m_shader;

    uint32_t m_vbo{};
};
} // namespace Peach
