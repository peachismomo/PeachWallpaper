/**
 * @file src/Renderer/EGL/GLES.hpp
 * @brief GL_CALL macro: clears the GL error queue before each call and
 * checks it afterward, logging the call site if it failed.
 */
#pragma once

#include <GLES2/gl2.h>
#include <csignal>
#include <string>

#include "ur_log/ur-log.h"

#ifndef NDEBUG
#define DEBUG_BREAK() raise(SIGTRAP)
#define GLASSERT(x)                                                            \
    do {                                                                       \
        if (!(x))                                                              \
            DEBUG_BREAK();                                                     \
    } while (0)
#else
#define GLASSERT(x)                                                            \
    do {                                                                       \
        if (!(x))                                                              \
            ((void)0);                                                         \
    } while (0)
#endif

#ifndef NDEBUG
#define GL_CALL(x)                                                             \
    do {                                                                       \
        ClearOGLError();                                                       \
        x;                                                                     \
        GLASSERT(LogOGLCall(#x, __FILE__, __LINE__));                          \
    } while (0)

inline void ClearOGLError() {
    while (glGetError() != GL_NO_ERROR)
        ;
}

inline bool LogOGLCall(const char *function, const char *file, int line) {
    while (GLenum err = glGetError()) {
        std::string err_name;
        switch (err) {
        case GL_INVALID_ENUM:
            err_name = "GL_INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            err_name = "GL_INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            err_name = "GL_INVALID_OPERATION";
            break;
        case GL_OUT_OF_MEMORY:
            err_name = "GL_OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            err_name = "GL_INVALID_FRAMEBUFFER_OPERATION";
            break;
        default:
            err_name = "UNKNOWN ERROR";
            break;
        }
        UR_ERROR("[OpenGL Error]: {}\nFunction: {}\nFile: {}, line {}",
                 err_name, function, file, line);
        return false;
    }
    return true;
}
#else
#define GL_CALL(x)                                                             \
    do {                                                                       \
        x;                                                                     \
    } while (0)
#endif
