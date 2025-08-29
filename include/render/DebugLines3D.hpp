#pragma once
#include <glad/glad.h>
#include <glm/vec3.hpp>

class DebugLines3D
{
public:
	bool init();
	void setTriangle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);
	void clear();
	//issues GL_LINES for 3 edges if set
	void draw() const; 
	void destroy();

private: 
	GLuint vao_ = 0;
	GLuint vbo_ = 0;
	bool has_ = false;
};