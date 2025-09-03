// include/gfx/SpriteBatch.hpp
#pragma once
#include <glad/glad.h>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include "gfx/Shader.hpp"
#include <glm/mat4x4.hpp> 

struct Sprite 
{
    glm::vec2 pos;     // bottom-left in pixels
    glm::vec2 size;    // width/height in pixels
    glm::vec4 uv;      // (u0, v0, u1, v1) in 0..1
    glm::vec4 color;   // RGBA (0..1)
};

class SpriteBatch 
{
public:
    bool init(const char* vsPath, const char* fsPath, const char* texturePath, int maxSprites = 2000);
    void shutdown();

    // Begin a new frame; provide framebuffer size (for ortho)
    void begin(int fbw, int fbh);

    // Queue sprites (CPU only). You can call this many times per frame.
    void push(const Sprite& s);

    // Upload CPU data to GPU and issue ONE draw call
    void endAndDraw();

    // Convenience
    void setTexture(GLuint tex); // if you want to swap texture later
    void beginWithVP(const glm::mat4& VP);
    void setSampleMode(int mode); // 0 = normal, 1 = font mask
    GLuint texture() const { return m_tex; }

private:
    struct Vertex 
    {
        float x, y;    // position in pixels
        float u, v;    // uv
        float r, g, b, a; // color
    };

    bool loadTexture(const char* path);

    GLuint m_vao = 0;
    GLuint m_vbo = 0;     // dynamic (vertices)
    GLuint m_ebo = 0;     // static (indices)
    GLuint m_tex = 0;

    ShaderProgram m_prog;
    GLint m_uP = -1;
    GLint m_uTex = -1;
    GLint m_uMode = -1;

    int m_maxSprites = 0;
    int m_spriteCount = 0;

    // CPU staging buffers (resized to capacity once)
    std::vector<Vertex> m_cpuVerts;   // 4 verts per sprite
    std::vector<unsigned int> m_cpuIndices;// 6 indices per sprite
};
