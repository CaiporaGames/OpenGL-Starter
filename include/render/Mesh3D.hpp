#pragma once
#include <glad/glad.h>

class Mesh3D
{
public:
	bool initColoredCube();
	void draw() const;
	void destroy();
private:
	GLuint vao_ = 0;
	GLuint vbo_ = 0;
	GLuint ebo_ = 0;
	GLsizei indexCount_ = 0;
};