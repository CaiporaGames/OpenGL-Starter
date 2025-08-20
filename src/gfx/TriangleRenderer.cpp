#include "gfx/TriangleRenderer.hpp"
#include <array>
#include <cstddef>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // translate, scale, ortho
#include <glm/gtc/type_ptr.hpp>

#include "thirdparty/stb_image.h"

// Unit quad (model space) with UVs
// pos: (0..1), uv: (0..1)
static constexpr std::array<float, 4 * (2 + 2)> kVertices = {
    //  x,   y,   u,   v
     0.0f, 0.0f, 0.0f, 0.0f, // 0 bottom-left
     1.0f, 0.0f, 1.0f, 0.0f, // 1 bottom-right
     0.0f, 1.0f, 0.0f, 1.0f, // 2 top-left
     1.0f, 1.0f, 1.0f, 1.0f  // 3 top-right
};

static constexpr std::array<unsigned int, 6> kIndices = { 0, 1, 2, 2, 1, 3 };

bool TriangleRenderer::init(const char* vsPath, const char* fsPath, const char* texturePath) {
    // 1) Program
    if (!m_prog.loadFromFiles(vsPath, fsPath)) return false;
    m_uMVP = m_prog.uniformLocation("u_MVP");
    m_uTex = m_prog.uniformLocation("uTex");

    // 2) Buffers
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kVertices), kVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kIndices), kIndices.data(), GL_STATIC_DRAW);

    // Attributes: stride = (2 pos + 2 uv) * float
    constexpr GLsizei stride = static_cast<GLsizei>((2 + 2) * sizeof(float));

    // aPos = location 0
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(0));

    // aUV = location 1 (offset = 2 floats)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(2 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // 3) Texture
    if (!loadTexture(texturePath)) return false;

    return true;
}

bool TriangleRenderer::loadTexture(const char* path) {
    int w = 0, h = 0, comp = 0;
    stbi_set_flip_vertically_on_load(true); // PNGs are usually top-left origin
    unsigned char* pixels = stbi_load(path, &w, &h, &comp, 0);
    if (!pixels) {
        std::cerr << "[Texture] Failed to load: " << path << "\n";
        return false;
    }

    GLenum format = GL_RGBA;
    if (comp == 1) format = GL_RED;
    else if (comp == 3) format = GL_RGB;
    else if (comp == 4) format = GL_RGBA;

    // If row alignment isn’t multiple of 4, fix unpack alignment
    GLint prevUnpack = 0;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &prevUnpack);
    if ((w * comp) % 4 != 0) glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glGenTextures(1, &m_tex);
    glBindTexture(GL_TEXTURE_2D, m_tex);

    // Basic sampling params (sprite-friendly)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // smooth + mipmaps
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);               // smooth
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);            // avoid bleeding
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Upload
    glTexImage2D(GL_TEXTURE_2D, 0,
        (format == GL_RGBA || format == GL_RGB) ? (format == GL_RGBA ? GL_RGBA8 : GL_RGB8) : GL_R8,
        w, h, 0, format, GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Restore unpack alignment
    if ((w * comp) % 4 != 0) glPixelStorei(GL_UNPACK_ALIGNMENT, prevUnpack);

    stbi_image_free(pixels);
    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
}

void TriangleRenderer::draw(int fbw, int fbh) {
    m_prog.use();
    glBindVertexArray(m_vao);

    // Bind texture to unit 0 and set sampler
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_tex);
    if (m_uTex != -1) glUniform1i(m_uTex, 0);

    // Build orthographic MVP in **pixel units**:
    // (0,0) at bottom-left, (fbw, fbh) at top-right
    glm::mat4 P = glm::ortho(0.0f, float(fbw), 0.0f, float(fbh), -1.0f, 1.0f);
    glm::mat4 V(1.0f);

    // Model transforms the unit quad to [pos .. pos+size] in pixels
    glm::mat4 M(1.0f);
    M = glm::translate(M, glm::vec3(m_posX, m_posY, 0.0f));
    M = glm::scale(M, glm::vec3(m_sizeX, m_sizeY, 1.0f));

    glm::mat4 MVP = P * V * M;

    if (m_uMVP != -1) {
        glUniformMatrix4fv(m_uMVP, 1, GL_FALSE, glm::value_ptr(MVP));
    }

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void TriangleRenderer::shutdown() {
    if (m_tex) { glDeleteTextures(1, &m_tex); m_tex = 0; }
    if (m_ebo) { glDeleteBuffers(1, &m_ebo); m_ebo = 0; }
    if (m_vbo) { glDeleteBuffers(1, &m_vbo); m_vbo = 0; }
    if (m_vao) { glDeleteVertexArrays(1, &m_vao); m_vao = 0; }
    m_prog.destroy();
}
