#include "gfx/GLDebug.hpp"
#include <glad/glad.h>
#include <cstdio>

namespace {
    // Basic readable mapping; extend if you like.
    const char* srcName(GLenum s) {
        switch (s) {
        case GL_DEBUG_SOURCE_API:             return "API";
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   return "WindowSys";
        case GL_DEBUG_SOURCE_SHADER_COMPILER: return "Shader";
        case GL_DEBUG_SOURCE_THIRD_PARTY:     return "3rdParty";
        case GL_DEBUG_SOURCE_APPLICATION:     return "App";
        case GL_DEBUG_SOURCE_OTHER:           return "Other";
        default: return "?";
        }
    }
    const char* typeName(GLenum t) {
        switch (t) {
        case GL_DEBUG_TYPE_ERROR:               return "Error";
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "Deprecated";
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  return "Undef";
        case GL_DEBUG_TYPE_PORTABILITY:         return "Portability";
        case GL_DEBUG_TYPE_PERFORMANCE:         return "Perf";
        case GL_DEBUG_TYPE_MARKER:              return "Marker";
        case GL_DEBUG_TYPE_PUSH_GROUP:          return "PushGrp";
        case GL_DEBUG_TYPE_POP_GROUP:           return "PopGrp";
        default: return "?";
        }
    }
    const char* sevName(GLenum s) {
        switch (s) {
        case GL_DEBUG_SEVERITY_HIGH:         return "HIGH";
        case GL_DEBUG_SEVERITY_MEDIUM:       return "MED";
        case GL_DEBUG_SEVERITY_LOW:          return "LOW";
        case GL_DEBUG_SEVERITY_NOTIFICATION: return "NOTE";
        default: return "?";
        }
    }

    void APIENTRY cb(GLenum source, GLenum type, GLuint id, GLenum severity,
        GLsizei, const GLchar* message, const void*) {
        // Filter noisy ids here if desired.
        if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) return; // keep quiet by default
        std::fprintf(stderr, "[GL %s|%s|%s id=%u] %s\n",
            srcName(source), typeName(type), sevName(severity), id, message);
    }
}

void GLDebug::enable() {
#if !defined(NDEBUG)
    // Available either via GL 4.3+ core or KHR_debug on 3.3 contexts.
    if (GLAD_GL_KHR_debug || GLAD_GL_ARB_debug_output) 
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(cb, nullptr);
        // Optional: silence notifications globally (kept also in callback early-return)
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE,
            GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
        std::fprintf(stderr, "[GLDebug] Debug output enabled.\n");
    }
    else {
        std::fprintf(stderr, "[GLDebug] KHR_debug not available (ok).\n");
    }
#endif
}
