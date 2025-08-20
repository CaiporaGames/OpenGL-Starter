#include "gfx/Shader.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

bool ShaderProgram::readTextFile(const char* path, std::string& out) {
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        std::cerr << "[Shader] Failed to open file: " << path << "\n";
        return false;
    }
    std::ostringstream ss;
    ss << f.rdbuf();
    out = ss.str();
    return true;
}

bool ShaderProgram::compile(GLenum type, const std::string& src, GLuint& outShader, std::string& log) {
    outShader = glCreateShader(type);
    const char* ptr = src.c_str();
    glShaderSource(outShader, 1, &ptr, nullptr);
    glCompileShader(outShader);

    GLint ok = GL_FALSE;
    glGetShaderiv(outShader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetShaderiv(outShader, GL_INFO_LOG_LENGTH, &len);
        std::vector<GLchar> buf(static_cast<size_t>(len > 1 ? len : 1));
        glGetShaderInfoLog(outShader, len, nullptr, buf.data());
        log.assign(buf.begin(), buf.end());
        glDeleteShader(outShader);
        outShader = 0;
        return false;
    }
    return true;
}

bool ShaderProgram::link(GLuint prog, GLuint vs, GLuint fs, std::string& log) {
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);

    GLint ok = GL_FALSE;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
        std::vector<GLchar> buf(static_cast<size_t>(len > 1 ? len : 1));
        glGetProgramInfoLog(prog, len, nullptr, buf.data());
        log.assign(buf.begin(), buf.end());
        return false;
    }
    return true;
}

bool ShaderProgram::loadFromFiles(const char* vsPath, const char* fsPath) {
    // Clear any previous program
    destroy();

    std::string vsrc, fsrc;
    if (!readTextFile(vsPath, vsrc)) return false;
    if (!readTextFile(fsPath, fsrc)) return false;

    GLuint vs = 0, fs = 0;
    std::string log;

    if (!compile(GL_VERTEX_SHADER, vsrc, vs, log)) {
        std::cerr << "[Shader] Vertex compile error (" << vsPath << "):\n" << log << "\n";
        return false;
    }
    if (!compile(GL_FRAGMENT_SHADER, fsrc, fs, log)) {
        std::cerr << "[Shader] Fragment compile error (" << fsPath << "):\n" << log << "\n";
        if (vs) glDeleteShader(vs);
        return false;
    }

    m_id = glCreateProgram();
    if (!link(m_id, vs, fs, log)) {
        std::cerr << "[Shader] Link error:\n" << log << "\n";
        glDeleteShader(vs);
        glDeleteShader(fs);
        glDeleteProgram(m_id);
        m_id = 0;
        return false;
    }

    // Once linked, shader objects can be deleted.
    glDeleteShader(vs);
    glDeleteShader(fs);
    return true;
}

void ShaderProgram::destroy() {
    if (m_id) {
        glDeleteProgram(m_id);
        m_id = 0;
    }
}
