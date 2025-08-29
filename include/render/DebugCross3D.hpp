#pragma once
#include <glad/glad.h>
#include <glm/vec3.hpp>

class DebugCross3D 
{
public:
    bool init();
    void setCross(const glm::vec3& worldPos, float halfLenWorld); // size in world units
    void clear();
    void draw() const; // GL_LINES (6 verts)
    void destroy();
private:
    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    bool has_ = false;
};
