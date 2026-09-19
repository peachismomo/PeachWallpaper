#include "GLESShader.hpp"
#include "GLES.hpp"
#include "ur_log/ur-log.h"
#include <GLES2/gl2.h>
#include <filesystem>
#include <glm/gtc/type_ptr.hpp>
#include <iterator>

namespace Peach {
bool GLESShader::Load(const std::string &filepath) {
    if (!std::filesystem::is_directory(filepath)) {
        UR_ERROR("GLES shader input expected to be folder");
        return false;
    }

    GL_CALL(m_program_id = glCreateProgram());

    for (auto &file : std::filesystem::directory_iterator(filepath)) {
        if (!file.is_regular_file())
            continue;
        auto path = file.path();
        GLenum shader_type;
        if (path.extension() == ".vert")
            shader_type = GL_VERTEX_SHADER;
        if (path.extension() == ".frag")
            shader_type = GL_FRAGMENT_SHADER;
        bool ret = CreateShader(path, shader_type);

        if (!ret) {
            UR_ERROR("Failed to compile shader");
            glDeleteProgram(m_program_id);
            m_program_id = 0;
            return false;
        }
    }

    GL_CALL(glLinkProgram(m_program_id));

    GLint link_result;
    GL_CALL(glGetProgramiv(m_program_id, GL_LINK_STATUS, &link_result));

    if (GL_FALSE == link_result) {
        std::string log_str;
        int log_len;

        GL_CALL(glGetProgramiv(m_program_id, GL_INFO_LOG_LENGTH, &log_len));
        if (log_len > 0) {
            std::vector<char> log(log_len);
            int written_log_len;
            GL_CALL(glGetProgramInfoLog(m_program_id, log_len, &written_log_len,
                                        log.data()));
            log_str += log.data();
        }
        UR_ERROR(log_str.c_str());

        GL_CALL(glDeleteProgram(m_program_id));
        m_program_id = 0;

        return false;
    }
    return true;
}

bool GLESShader::CreateShader(const std::filesystem::path &shader_filepath,
                              GLenum shader_type) {

    std::ifstream file(shader_filepath);
    if (!file.is_open()) {
        UR_ERROR("Failed to open shader file: {}",
                 shader_filepath.generic_string());
        return false;
    }

    std::string shader_src((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());

    uint32_t shader_id{};
    GL_CALL(shader_id = glCreateShader(shader_type));

    const char *src[] = {shader_src.c_str()};
    GL_CALL(glShaderSource(shader_id, 1, src, NULL));
    GL_CALL(glCompileShader(shader_id));

    int result{};

    GL_CALL(glGetShaderiv(shader_id, GL_COMPILE_STATUS, &result));

    if (result == false) {
        std::string log_string = "Shader compilation failed\n";
        int log_len;
        GL_CALL(glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_len));
        if (log_len > 0) {
            std::vector<char> log(log_len);
            int written_log_len;
            GL_CALL(glGetShaderInfoLog(shader_id, log_len, &written_log_len,
                                       log.data()));
            log_string += log.data();
        }

        UR_ERROR(log_string.c_str());
        GL_CALL(glDeleteShader(shader_id));
        return false;
    }

    GL_CALL(glAttachShader(m_program_id, shader_id));
    GL_CALL(glDeleteShader(shader_id));
    return true;
}

void GLESShader::Bind() { GL_CALL(glUseProgram(m_program_id)); }

int GLESShader::GetAttribLocation(const std::string &name) {
    GLint location{};
    GL_CALL(location = glGetAttribLocation(m_program_id, name.c_str()));
    if (location == -1)
        UR_WARN("Attribute '{}' not found in shader", name);
    return location;
}

GLint GLESShader::GetUniformLocation(const std::string &name) {
    GLint location{};
    GL_CALL(location = glGetUniformLocation(m_program_id, name.c_str()));
    if (location == -1)
        UR_WARN("Uniform '{}' not found in shader", name);
    return location;
}

void GLESShader::SetInt(const std::string &name, int value) {
    GLint location = GetUniformLocation(name);
    if (location == -1)
        return;
    GL_CALL(glUniform1i(location, value));
}

void GLESShader::SetFloat(const std::string &name, float value) {
    GLint location = GetUniformLocation(name);
    if (location == -1)
        return;
    GL_CALL(glUniform1f(location, value));
}

void GLESShader::SetVec2(const std::string &name, const glm::vec2 &value) {
    GLint location = GetUniformLocation(name);
    if (location == -1)
        return;
    GL_CALL(glUniform2fv(location, 1, glm::value_ptr(value)));
}

void GLESShader::SetVec3(const std::string &name, const glm::vec3 &value) {
    GLint location = GetUniformLocation(name);
    if (location == -1)
        return;
    GL_CALL(glUniform3fv(location, 1, glm::value_ptr(value)));
}

void GLESShader::SetVec4(const std::string &name, const glm::vec4 &value) {
    GLint location = GetUniformLocation(name);
    if (location == -1)
        return;
    GL_CALL(glUniform4fv(location, 1, glm::value_ptr(value)));
}

void GLESShader::Destroy() {
    if (!m_program_id)
        return;
    GL_CALL(glDeleteProgram(m_program_id));
    m_program_id = 0;
}

} // namespace Peach
