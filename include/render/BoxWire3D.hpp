#pragma once
#include <glad/glad.h>
#include <glm/vec3.hpp>

class BoxWire3D
{
public:
	bool init();
	//world space box
	void set(const glm::vec3& bmin, const glm::vec3& bmax);
	void draw() const;
	void destroy();

private:
	GLuint vao_ = 0;
	GLuint vbo_ = 0;//24 vertices = 12 edges * 2
	bool ready_ = false;
};