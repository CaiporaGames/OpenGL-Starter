#include "render/DebugCross3D.hpp"

bool DebugCross3D::init() 
{
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * 6, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0); glBindBuffer(GL_ARRAY_BUFFER, 0);
    has_ = false; return true;
}

void DebugCross3D::setCross(const glm::vec3& p, float h) 
{
    const float data[18] = 
    {
        p.x - h,p.y,p.z,  p.x + h,p.y,p.z,  // X
        p.x,p.y - h,p.z,  p.x,p.y + h,p.z,  // Y
        p.x,p.y,p.z - h,  p.x,p.y,p.z + h   // Z
    };

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(data), data);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    has_ = true;
}
void DebugCross3D::clear() { has_ = false; }

void DebugCross3D::draw() const 
{
    if (!has_) return;

    glBindVertexArray(vao_);
    glDrawArrays(GL_LINES, 0, 6);
    glBindVertexArray(0);
}
void DebugCross3D::destroy() 
{
    if (vbo_) glDeleteBuffers(1, &vbo_), vbo_ = 0;
    if (vao_) glDeleteVertexArrays(1, &vao_), vao_ = 0;

    has_ = false;
}
