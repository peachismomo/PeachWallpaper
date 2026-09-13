#pragma once

#include "../Shader.hpp"

#include "GLES.hpp"

#include <cstdint>
#include <filesystem>

namespace Peach {
class GLESShader : public Shader {
  public:
    GLESShader() = default;
    ~GLESShader() override { Destroy(); }

    GLESShader(const GLESShader &) = delete;
    GLESShader &operator=(const GLESShader &) = delete;
    GLESShader(GLESShader &&) = delete;
    GLESShader &operator=(GLESShader &&) = delete;

    bool Load(const std::string &filepath) override;
    void Bind() override;

    int GetAttribLocation(const std::string &name) override;

    void SetInt(const std::string &name, int value) override;
    void SetFloat(const std::string &name, float value) override;
    void SetVec2(const std::string &name, const glm::vec2 &value) override;
    void SetVec3(const std::string &name, const glm::vec3 &value) override;
    void SetVec4(const std::string &name, const glm::vec4 &value) override;

    void Destroy() override;

  private:
    bool CreateShader(const std::filesystem::path &shader_src,
                      GLenum shader_type);

    /** @brief Looks up a uniform's location, warning if it's not found in
     * the linked program. Returns -1 either way, matching
     * glGetUniformLocation's own convention. */
    GLint GetUniformLocation(const std::string &name);

    uint32_t m_program_id{};
};
} // namespace Peach
