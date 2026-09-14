#pragma once

#include <glm/glm.hpp>

#include <string>
#include <vector>

namespace Peach {
class Shader {
  public:
    struct IntUniform {
        std::string name{};
        int value{};
    };

    struct FloatUniform {
        std::string name{};
        float value{};
    };

    struct Vec2Uniform {
        std::string name{};
        glm::vec2 value{};
    };

    struct Vec3Uniform {
        std::string name{};
        glm::vec3 value{};
    };

    struct Vec4Uniform {
        std::string name{};
        glm::vec4 value{};
    };

    struct ShaderConfig {
        std::string shader_filepath{};
        std::vector<IntUniform> int_uniforms{};
        std::vector<FloatUniform> float_uniforms{};
        std::vector<Vec2Uniform> vec2_uniforms{};
        std::vector<Vec3Uniform> vec3_uniforms{};
        std::vector<Vec4Uniform> vec4_uniforms{};
    };

    /** @brief Creates an empty shader abstraction. */
    Shader() = default;

    /** @brief Releases resources owned by the concrete shader. */
    virtual ~Shader() = default;

    Shader(const Shader &) = delete;
    Shader &operator=(const Shader &) = delete;
    Shader(Shader &&) = delete;
    Shader &operator=(Shader &&) = delete;

    /** @brief Compiles and links the shader program from the given file. */
    virtual bool Load(const std::string &filepath) = 0;

    /** @brief Uses this shader program for subsequent draw calls. */
    virtual void Bind() = 0;

    /** @brief Looks up a vertex attribute's location by name. Returns -1 if
     * not found (e.g. optimized out, or the name doesn't match the shader
     * source). GLSL ES 1.00 has no layout(location=N) qualifier, so this
     * must be queried after linking rather than assumed. */
    virtual int GetAttribLocation(const std::string &name) = 0;

    /** @brief Sets an integer uniform by name. */
    virtual void SetInt(const std::string &name, int value) = 0;

    /** @brief Sets a floating-point uniform by name. */
    virtual void SetFloat(const std::string &name, float value) = 0;

    /** @brief Sets a two-component vector uniform by name. */
    virtual void SetVec2(const std::string &name, const glm::vec2 &value) = 0;

    /** @brief Sets a three-component vector uniform by name. */
    virtual void SetVec3(const std::string &name, const glm::vec3 &value) = 0;

    /** @brief Sets a four-component vector uniform by name. */
    virtual void SetVec4(const std::string &name, const glm::vec4 &value) = 0;

    /** @brief Releases GPU resources owned by this shader. */
    virtual void Destroy() = 0;

    /** @brief Stores the configuration used to initialize this shader. */
    void SetShaderConfig(const ShaderConfig &config) {
        m_shader_config = config;
    }

    /** @brief Returns the stored shader configuration. */
    ShaderConfig GetShaderConfig() const { return m_shader_config; }

  protected:
    ShaderConfig m_shader_config;
};
} // namespace Peach
