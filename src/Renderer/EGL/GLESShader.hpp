#pragma once

#include "../Shader.hpp"

#include "GLES.hpp"

#include <cstdint>
#include <filesystem>

namespace Peach {
class GLESShader : public Shader {
  public:
    /** @brief Creates an empty shader program wrapper. */
    GLESShader() = default;

    /** @brief Releases the linked OpenGL ES program. */
    ~GLESShader() override { Destroy(); }

    GLESShader(const GLESShader &) = delete;
    GLESShader &operator=(const GLESShader &) = delete;
    GLESShader(GLESShader &&) = delete;
    GLESShader &operator=(GLESShader &&) = delete;

    /** @brief Compiles shader files in a directory and links a program. */
    bool Load(const std::string &filepath) override;

    /** @brief Binds the linked program for subsequent draw calls. */
    void Bind() override;

    /** @brief Returns the location of a vertex attribute in the program. */
    int GetAttribLocation(const std::string &name) override;

    /** @brief Sets an integer uniform when the named uniform exists. */
    void SetInt(const std::string &name, int value) override;

    /** @brief Sets a floating-point uniform when the named uniform exists. */
    void SetFloat(const std::string &name, float value) override;

    /** @brief Sets a two-component vector uniform when it exists. */
    void SetVec2(const std::string &name, const glm::vec2 &value) override;

    /** @brief Sets a three-component vector uniform when it exists. */
    void SetVec3(const std::string &name, const glm::vec3 &value) override;

    /** @brief Sets a four-component vector uniform when it exists. */
    void SetVec4(const std::string &name, const glm::vec4 &value) override;

    /** @brief Deletes the linked OpenGL ES program, if one exists. */
    void Destroy() override;

  private:
    /** @brief Compiles and attaches one shader source file to the program. */
    bool CreateShader(const std::filesystem::path &shader_src,
                      GLenum shader_type);

    /** @brief Looks up a uniform's location, warning if it's not found in
     * the linked program. Returns -1 either way, matching
     * glGetUniformLocation's own convention. */
    GLint GetUniformLocation(const std::string &name);

    uint32_t m_program_id{};
};
} // namespace Peach
