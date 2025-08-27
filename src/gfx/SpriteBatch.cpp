#include "gfx/SpriteBatch.hpp"
#include <cassert>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // ortho
#include <glm/gtc/type_ptr.hpp>

#include "thirdparty/stb_image.h"

bool SpriteBatch::init(const char* vsPath, const char* fsPath, const char* texturePath, int maxSprites) 
{
    m_maxSprites = maxSprites;
    m_spriteCount = 0;

    // 1) Program + uniforms
    if (!m_prog.loadFromFiles(vsPath, fsPath)) return false;

    m_uP = m_prog.uniformLocation("u_P");
    m_uTex = m_prog.uniformLocation("uTex");
    m_uMode = m_prog.uniformLocation("u_Mode");

    // 2) CPU buffers sized to capacity
    m_cpuVerts.resize(static_cast<size_t>(m_maxSprites) * 4);
    m_cpuIndices.resize(static_cast<size_t>(m_maxSprites) * 6);

    // Precompute EBO indices once: [0..3] for each sprite
    for (int i = 0; i < m_maxSprites; ++i) 
    {
        unsigned int baseV = static_cast<unsigned int>(i) * 4u;
        unsigned int baseI = static_cast<unsigned int>(i) * 6u;
        m_cpuIndices[baseI + 0] = baseV + 0;
        m_cpuIndices[baseI + 1] = baseV + 1;
        m_cpuIndices[baseI + 2] = baseV + 2;
        m_cpuIndices[baseI + 3] = baseV + 2;
        m_cpuIndices[baseI + 4] = baseV + 1;
        m_cpuIndices[baseI + 5] = baseV + 3;
    }

    // 3) GL objects
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);

    // VBO: allocate enough for maxSprites * 4 vertices (dynamic)
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(m_cpuVerts.size() * sizeof(Vertex)),
        nullptr, // no data yet
        GL_DYNAMIC_DRAW);

    // EBO: upload static index table
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(m_cpuIndices.size() * sizeof(unsigned int)),
        m_cpuIndices.data(),
        GL_STATIC_DRAW);

    // Vertex layout: pos(2), uv(2), color(4) — all floats
    const GLsizei stride = static_cast<GLsizei>(sizeof(Vertex));
    // aPos
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(offsetof(Vertex, x)));
    // aUV
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(offsetof(Vertex, u)));
    // aColor
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(offsetof(Vertex, r)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // 4) Texture
    if (!loadTexture(texturePath)) return false;

    return true;
}

bool SpriteBatch::loadTexture(const char* path) {
    int w = 0, h = 0, comp = 0;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* pixels = stbi_load(path, &w, &h, &comp, 0);
    if (!pixels) {
        std::cerr << "[SpriteBatch] Failed to load texture: " << path << "\n";
        return false;
    }

    GLenum format = (comp == 4) ? GL_RGBA : (comp == 3) ? GL_RGB : GL_RED;

    GLint prevUnpack = 0;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &prevUnpack);
    if ((w * comp) % 4 != 0) glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glGenTextures(1, &m_tex);
    glBindTexture(GL_TEXTURE_2D, m_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0,
        (format == GL_RGBA) ? GL_RGBA8 : (format == GL_RGB) ? GL_RGB8 : GL_R8,
        w, h, 0, format, GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    if ((w * comp) % 4 != 0) glPixelStorei(GL_UNPACK_ALIGNMENT, prevUnpack);

    stbi_image_free(pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

void SpriteBatch::shutdown() {
    if (m_tex) glDeleteTextures(1, &m_tex), m_tex = 0;
    if (m_ebo) glDeleteBuffers(1, &m_ebo), m_ebo = 0;
    if (m_vbo) glDeleteBuffers(1, &m_vbo), m_vbo = 0;
    if (m_vao) glDeleteVertexArrays(1, &m_vao), m_vao = 0;
    m_prog.destroy();
}

void SpriteBatch::begin(int fbw, int fbh) {
    m_spriteCount = 0;

    // Set projection once per frame
    m_prog.use();
    if (m_uP != -1) {
        glm::mat4 P = glm::ortho(0.0f, float(fbw), 0.0f, float(fbh), -1.0f, 1.0f);
        glUniformMatrix4fv(m_uP, 1, GL_FALSE, glm::value_ptr(P));
    }
    // Bind sampler to unit 0
    if (m_uTex != -1) glUniform1i(m_uTex, 0);
}

void SpriteBatch::push(const Sprite& s) {
    if (m_spriteCount >= m_maxSprites) return; // silently drop if overflow (or grow)

    const unsigned int i = static_cast<unsigned int>(m_spriteCount);
    const float x = s.pos.x;
    const float y = s.pos.y;
    const float w = s.size.x;
    const float h = s.size.y;

    const float u0 = s.uv.x, v0 = s.uv.y, u1 = s.uv.z, v1 = s.uv.w;
    const float r = s.color.r, g = s.color.g, b = s.color.b, a = s.color.a;

    // 4 vertices for this sprite in CPU buffer
    Vertex* v = &m_cpuVerts[i * 4u];
    // bottom-left
    v[0] = { x,     y,     u0, v0, r, g, b, a };
    // bottom-right
    v[1] = { x + w, y,     u1, v0, r, g, b, a };
    // top-left
    v[2] = { x,     y + h, u0, v1, r, g, b, a };
    // top-right
    v[3] = { x + w, y + h, u1, v1, r, g, b, a };

    ++m_spriteCount;
}

void SpriteBatch::endAndDraw() 
{
    if (m_spriteCount == 0) return;

    // Upload only what we used this frame
    const GLsizeiptr bytes = static_cast<GLsizeiptr>(m_spriteCount * 4 * sizeof(Vertex));
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, bytes, m_cpuVerts.data()); // dynamic update

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_tex);

    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, m_spriteCount * 6, GL_UNSIGNED_INT, (void*)0);

    // cleanup bindings
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void SpriteBatch::setTexture(GLuint tex) { m_tex = tex; }

void SpriteBatch::beginWithVP(const glm::mat4& VP) 
{
    m_spriteCount = 0;
    m_prog.use();
    if (m_uP != -1) glUniformMatrix4fv(m_uP, 1, GL_FALSE, glm::value_ptr(VP));
    if (m_uTex != -1) glUniform1i(m_uTex, 0);
    if (m_uMode != -1) glUniform1i(m_uMode, 0); // default: normal RGBA
}

 void SpriteBatch::setSampleMode(int mode) 
 {
    m_prog.use();
    if (m_uMode != -1) glUniform1i(m_uMode, mode);
}
