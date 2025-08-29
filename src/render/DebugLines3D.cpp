#include "render/DebugLines3D.hpp"

bool DebugLines3D::init()
{
	glGenVertexArrays(1, &vao_);
	glGenBuffers(1, &vbo_);

	glBindVertexArray(vao_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*6, nullptr, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), NULL);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	has_ = false;
	return true;
}

void DebugLines3D::setTriangle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
{
	const float data[18] =
	{
		a.x,a.y,a.z,  b.x,b.y,b.z, // ab
		b.x,b.y,b.z,  c.x,c.y,c.z, // bc
		c.x,c.y,c.z,  a.x,a.y,a.z  // ca
	};
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(data), data);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	has_ = true;
}


void DebugLines3D::clear()
{
	has_ = false;
}

void DebugLines3D::draw() const
{
	if (!has_) return;

	glBindVertexArray(vao_);
	glDrawArrays(GL_LINES, 0, 6);
	glBindVertexArray(0);
}

void DebugLines3D::destroy()
{
	if (vbo_) glDeleteBuffers(1, &vbo_), vbo_ = 0;
	if (vao_) glDeleteBuffers(1, &vao_), vao_ = 0;

	has_ = false;
}