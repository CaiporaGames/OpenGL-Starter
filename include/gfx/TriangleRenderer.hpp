#pragma once
#include <glad/glad.h>
#include "gfx/Shader.hpp"

class TriangleRenderer {
public:
    // Now accepts a texture path as well as shader files
    bool init(const char* vsPath, const char* fsPath, const char* texturePath);
    void draw(int fbw, int fbh); // needs framebuffer size for ortho
    void shutdown();

    // Optional: simple setters to move/scale the sprite
    void setPosition(float x, float y) { m_posX = x; m_posY = y; }
    void setSize(float w, float h) { m_sizeX = w; m_sizeY = h; }

private:
    bool loadTexture(const char* path);

    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLuint m_ebo = 0;
    GLuint m_tex = 0;

    ShaderProgram m_prog;
    GLint m_uMVP = -1;
    GLint m_uTex = -1;

    // sprite placement in pixels
    float m_posX = 50.0f;
    float m_posY = 50.0f;
    float m_sizeX = 256.0f; // default: 256×256 sprite
    float m_sizeY = 256.0f;
};
