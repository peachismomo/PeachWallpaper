#include "GLESShaderRenderable.hpp"

#include "../GlmGlaze.hpp"

#include <GLES2/gl2.h>
#include <cstddef>
#include <glaze/json/read.hpp>
#include <stdexcept>

#include "GLES.hpp"

namespace Peach {
GLESShaderRenderable::GLESShaderRenderable(const std::string &config)
    : Renderable(config) {
    Shader::ShaderConfig shader_config;
    std::string buffer;
    auto ec = glz::read_file_json(shader_config, config, buffer);
    if (ec) {
        throw std::runtime_error("Failed to load shader renderable config: " +
                                 glz::format_error(ec, buffer));
    }

    if (!m_shader.Load(shader_config.shader_filepath)) {
        throw std::runtime_error("Failed to load shader: " +
                                 shader_config.shader_filepath);
    }

    m_shader.SetShaderConfig(shader_config);

    GL_CALL(glGenBuffers(1, &m_vbo));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, m_vbo));

    std::vector<Vertex> quad = {
        {{-1, -1}, {0, 0}}, {{1, -1}, {1, 0}}, {{1, 1}, {1, 1}},
        {{-1, -1}, {0, 0}}, {{1, 1}, {1, 1}},  {{-1, 1}, {0, 1}},
    };
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * quad.size(),
                         quad.data(), GL_STATIC_DRAW));

    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void GLESShaderRenderable::Update(float time) { m_time += time; }

void GLESShaderRenderable::Draw() {
    m_shader.Bind();
    m_shader.SetFloat("u_time", m_time);

    for (const auto &uniform : m_shader.GetShaderConfig().int_uniforms) {
        m_shader.SetInt(uniform.name, uniform.value);
    }

    for (const auto &uniform : m_shader.GetShaderConfig().float_uniforms) {
        m_shader.SetFloat(uniform.name, uniform.value);
    }

    for (const auto &uniform : m_shader.GetShaderConfig().vec2_uniforms) {
        m_shader.SetVec2(uniform.name, uniform.value);
    }

    for (const auto &uniform : m_shader.GetShaderConfig().vec3_uniforms) {
        m_shader.SetVec3(uniform.name, uniform.value);
    }

    for (const auto &uniform : m_shader.GetShaderConfig().vec4_uniforms) {
        m_shader.SetVec4(uniform.name, uniform.value);
    }

    int pos_loc = m_shader.GetAttribLocation("a_position");
    int uv_loc = m_shader.GetAttribLocation("a_uv");

    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, m_vbo));

    if (pos_loc != -1) {
        GL_CALL(glVertexAttribPointer(pos_loc, 2, GL_FLOAT, GL_FALSE,
                                      sizeof(Vertex),
                                      (void *)offsetof(Vertex, pos)));
        GL_CALL(glEnableVertexAttribArray(pos_loc));
    }

    if (uv_loc != -1) {
        GL_CALL(glVertexAttribPointer(uv_loc, 2, GL_FLOAT, GL_FALSE,
                                      sizeof(Vertex),
                                      (void *)offsetof(Vertex, uv)));
        GL_CALL(glEnableVertexAttribArray(uv_loc));
    }

    GL_CALL(glDrawArrays(GL_TRIANGLES, 0, 6));

    if (pos_loc != -1)
        GL_CALL(glDisableVertexAttribArray(pos_loc));
    if (uv_loc != -1)
        GL_CALL(glDisableVertexAttribArray(uv_loc));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void GLESShaderRenderable::Destroy() {
    m_shader.Destroy();

    if (m_vbo) {
        GL_CALL(glDeleteBuffers(1, &m_vbo));
        m_vbo = 0;
    }
}
} // namespace Peach
