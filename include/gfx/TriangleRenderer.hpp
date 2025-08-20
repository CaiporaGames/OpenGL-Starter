#pragma once
#include <glad/glad.h>
#include "gfx/Shader.hpp"

class TriangleRenderer {
public:
	bool init(const char* vsPath, const char* fsPath);
	void draw();
	void shutdown();

private:
	ShaderProgram prog_;
	GLuint vao_ = 0;
	GLuint vbo_ = 0;
};
