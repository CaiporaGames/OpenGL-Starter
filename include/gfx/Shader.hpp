#pragma once
#include <glad/glad.h>
#include <string>

class ShaderProgram {
public:
    ShaderProgram() = default;
    ~ShaderProgram() { destroy(); }

    // Compile + link from GLSL files
    bool loadFromFiles(const char* vsPath, const char* fsPath);

    // Bind program
    void use() const { glUseProgram(m_id); }

    // Get raw program id (optional)
    GLuint id() const { return m_id; }

    // Free GL objects if any
    void destroy();

    // Optional: uniform location helper
    GLint uniformLocation(const char* name) const {
        return glGetUniformLocation(m_id, name);
    }
    void setMat4(const char* name, const float* m) const {
        const GLint loc = glGetUniformLocation(m_id, name);
        if (loc != -1) glUniformMatrix4fv(loc, 1, GL_FALSE, m);
    }

private:
    GLuint m_id = 0;

    static bool readTextFile(const char* path, std::string& out);
    static bool compile(GLenum type, const std::string& src, GLuint& outShader, std::string& log);
    static bool link(GLuint prog, GLuint vs, GLuint fs, std::string& log);
};
