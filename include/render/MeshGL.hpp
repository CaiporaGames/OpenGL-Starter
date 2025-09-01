#pragma once
#include <glad/glad.h>
#include <vector>
#include <cstdint>
#include "core/mesh_data.hpp"

class MeshGL
{
public:
	MeshGL() = default;
	~MeshGL(){ destroy(); }

	bool upload(const core::MeshData& cpu);//positions + indices
	void draw() const;
	void destroy();

	//CPU access for picking and BVH
	const float* positions() const { return cpuPos_.empty() ? nullptr : cpuPos_.data(); }
	const unsigned* indices() const { return cpuIdx_.empty() ? nullptr : cpuIdx_.data(); }
	std::size_t triCount() const { return cpuIdx_.size() / 3; }
	std::size_t vertexCount() const { return cpuPos_.size() / 3; }

private:
	GLsizei indexCount_ = 0;
	GLuint ebo_ = 0;
	GLuint vao_ = 0;
	GLuint vbo_ = 0;
	std::vector<float> cpuPos_;
	std::vector<unsigned> cpuIdx_;
};