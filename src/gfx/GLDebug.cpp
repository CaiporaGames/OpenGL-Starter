#include "gfx/GLDebug.hpp"
#include <glad/glad.h>
#include <vector>
#include <mutex>
#include <cstdio>
#include <utility>

namespace {
    std::vector<GLDebug::Message> g_buf;
    std::mutex g_mx;
    bool g_enabled = false;
    bool g_verbose = true;

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
        case GL_DEBUG_TYPE_PUSH_GROUP:          return "Push";
        case GL_DEBUG_TYPE_POP_GROUP:           return "Pop";
        default: return "?";
        }
    }
    const char* sevName(GLenum s) {
        switch (s) {
        case GL_DEBUG_SEVERITY_HIGH:         return "High";
        case GL_DEBUG_SEVERITY_MEDIUM:       return "Medium";
        case GL_DEBUG_SEVERITY_LOW:          return "Low";
        case GL_DEBUG_SEVERITY_NOTIFICATION: return "Note";
        default: return "?";
        }
    }

    void APIENTRY cb(GLenum source, GLenum type, GLuint id,
        GLenum severity, GLsizei, const GLchar* message, const void*)
    {
        if (!g_verbose && severity == GL_DEBUG_SEVERITY_NOTIFICATION) return;

        GLDebug::Message m;
        m.source = source;
        m.type = type;
        m.id = id;
        m.severity = severity;
        m.text = message ? message : "";

        {
            std::lock_guard<std::mutex> lk(g_mx);
            g_buf.emplace_back(std::move(m));
        }

        // Also print to stdout for convenience
        std::printf("[GL %s/%s/%s #%u] %s\n",
            srcName(source), typeName(type), sevName(severity), id,
            message ? message : "");
    }
} // anon

namespace GLDebug {

    void enable(bool verbose)
    {
        g_verbose = verbose;

        // Works on GL 4.3+ or when KHR_debug is present. Check function pointer.
        if (glad_glDebugMessageCallback) {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(cb, nullptr);
            g_enabled = true;
        }
        else {
            g_enabled = false;
            std::puts("[GLDebug] KHR_debug not available; messages disabled.");
        }
    }

    void disable()
    {
        if (g_enabled && glad_glDebugMessageCallback) {
            glDebugMessageCallback(nullptr, nullptr);
            glDisable(GL_DEBUG_OUTPUT);
            glDisable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        }
        g_enabled = false;
    }

    void clear()
    {
        std::lock_guard<std::mutex> lk(g_mx);
        g_buf.clear();
    }

    const std::vector<Message>& messages()
    {
        return g_buf;
    }

} // namespace GLDebug
