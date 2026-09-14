#pragma once

#include "../Renderable.hpp"
#include "GLESShader.hpp"

#include <string>

namespace Peach {
class GLESShaderRenderable : public Renderable {
  public:
    /** @brief Loads a shader renderable configuration and GPU resources. */
    explicit GLESShaderRenderable(const std::string &config);

    /** @brief Releases the shader and vertex-buffer resources. */
    ~GLESShaderRenderable() override { Destroy(); }

    GLESShaderRenderable() = delete;
    GLESShaderRenderable(const GLESShaderRenderable &) = delete;
    GLESShaderRenderable &operator=(const GLESShaderRenderable &) = delete;
    GLESShaderRenderable(GLESShaderRenderable &&) = delete;
    GLESShaderRenderable &operator=(GLESShaderRenderable &&) = delete;

    /** @brief Advances the shader's elapsed-time uniform. */
    void Update(float time) override;

    /** @brief Binds the shader, uploads uniforms, and draws the quad. */
    void Draw() override;

    /** @brief Releases the shader and vertex-buffer resources. */
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
