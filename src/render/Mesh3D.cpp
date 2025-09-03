#include "render/Mesh3D.hpp"

bool Mesh3D::initColoredCube() {
    destroy();

    // 8 vertices (positions)
    static const float v[] = {
        -0.5f,-0.5f,-0.5f,  +0.5f,-0.5f,-0.5f,
        +0.5f,+0.5f,-0.5f,  -0.5f,+0.5f,-0.5f,
        -0.5f,-0.5f,+0.5f,  +0.5f,-0.5f,+0.5f,
        +0.5f,+0.5f,+0.5f,  -0.5f,+0.5f,+0.5f,
    };
    static const unsigned idx[] = {
        0,1,2,  2,3,0,
        5,4,7,  7,6,5,
        4,0,3,  3,7,4,
        1,5,6,  6,2,1,
        4,5,1,  1,0,4,
        3,2,6,  6,7,3,
    };

    cpuVerts_.assign(std::begin(v), std::end(v));
    cpuIdx_.assign(std::begin(idx), std::end(idx));
    indexCount_ = (GLsizei)(sizeof(idx) / sizeof(idx[0]));

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);

    glBindVertexArray(vao_);

    // VBO — you were missing this
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);

    // EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);

    // aPos
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return true;
}


void Mesh3D::draw() const
{
	glBindVertexArray(vao_);
	glDrawElements(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}

void Mesh3D::destroy() {
    if (ebo_) glDeleteBuffers(1, &ebo_), ebo_ = 0;
    if (vbo_) glDeleteBuffers(1, &vbo_), vbo_ = 0;
    if (vao_) glDeleteVertexArrays(1, &vao_), vao_ = 0;
}
