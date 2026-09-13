#pragma once

/**
 * @file GlmGlaze.hpp
 * @brief Teaches Glaze how to (de)serialize glm::vec2/3/4 as bare JSON
 * arrays (e.g. [1.0, 2.0, 3.0]). Glaze's automatic reflection only works on
 * plain aggregates; GLM's vector types have constructors, so without this
 * they fail to compile the moment something tries to parse a struct
 * containing one. Include this wherever a Glaze-parsed type embeds a glm
 * vector.
 */

#include <glaze/glaze.hpp>
#include <glm/glm.hpp>

template <> struct glz::meta<glm::vec2> {
    static constexpr auto value = glz::array(&glm::vec2::x, &glm::vec2::y);
};

template <> struct glz::meta<glm::vec3> {
    static constexpr auto value =
        glz::array(&glm::vec3::x, &glm::vec3::y, &glm::vec3::z);
};

template <> struct glz::meta<glm::vec4> {
    static constexpr auto value =
        glz::array(&glm::vec4::x, &glm::vec4::y, &glm::vec4::z, &glm::vec4::w);
};
