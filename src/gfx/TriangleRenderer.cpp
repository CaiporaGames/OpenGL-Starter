#include "gfx/TriangleRenderer.hpp"

bool TriangleRenderer::init(const char* vsPath, const char* fsPath) {
    if (!prog_.attachFromFile(GL_VERTEX_SHADER, vsPath)) return false;
    if (!prog_.attachFromFile(GL_FRAGMENT_SHADER, fsPath)) return false;
    if (!prog_.link()) return false;

    // interleaved: x,y,r,g,b
    const float verts[] = {
      -0.6f,-0.5f, 1.f,0.2f,0.2f,
       0.6f,-0.5f, 0.2f,1.f,0.2f,
       0.0f, 0.6f, 0.2f,0.2f,1.f
    };

    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);
    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    return true;
}

void TriangleRenderer::draw() {
    prog_.use();
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}

void TriangleRenderer::shutdown() {
    if (vbo_) { glDeleteBuffers(1, &vbo_); vbo_ = 0; }
    if (vao_) { glDeleteVertexArrays(1, &vao_); vao_ = 0; }
}
