#include "render/MeshGL.hpp"

bool MeshGL::upload(const core::MeshData& cpu)
{
	destroy();

	if (cpu.positions.empty() || cpu.indices.empty()) return false;

	cpuPos_.assign(cpu.positions.begin(), cpu.positions.end());
	cpuIdx_.assign(cpu.indices.begin(), cpu.indices.end());
	indexCount_ = static_cast<GLsizei>(cpuIdx_.size());

	// Generate and bind VAO
	glGenVertexArrays(1, &vao_);
	glBindVertexArray(vao_);

	// Generate and bind VBO
	glGenBuffers(1, &vbo_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	glBufferData(GL_ARRAY_BUFFER, cpuPos_.size() * sizeof(float), cpuPos_.data(), GL_STATIC_DRAW);

	// Configure vertex attribute (aPos)
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	// Generate and bind EBO
	glGenBuffers(1, &ebo_);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, cpuIdx_.size() * sizeof(uint32_t), cpuIdx_.data(), GL_STATIC_DRAW);

	// Unbind VAO and VBO
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return true;
}

void MeshGL::draw() const
{
	glBindVertexArray(vao_);
	glDrawElements(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, (void*)0);
	glBindVertexArray(0);
}

void MeshGL::destroy()
{
	if (ebo_) { glDeleteBuffers(1, &ebo_); ebo_ = 0; }
	if (vbo_) { glDeleteBuffers(1, &vbo_); vbo_ = 0; }
	if (vao_) { glDeleteVertexArrays(1, &vao_); vao_ = 0; }

	cpuPos_.clear();
	cpuIdx_.clear();
	indexCount_ = 0;
}