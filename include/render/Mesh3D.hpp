#pragma once
#include <glad/glad.h>
#include <vector>

class Mesh3D
{
public:
	bool initColoredCube();
	void draw() const;
	void destroy();

	//CPU data access for raycast (positions = xyz floats; indeces = uints)
	const float* cpuPositions() const
	{
		return cpuVerts_.empty() ? nullptr : cpuVerts_.data();
	}
	const unsigned* cpuIndices() const
	{
		return cpuIdx_.empty() ? nullptr : cpuIdx_.data();
	}
	std::size_t triCount() const
	{
		return cpuIdx_.size() / 3;
	}
private:
	GLuint vao_ = 0;
	GLuint vbo_ = 0;
	GLuint ebo_ = 0;
	GLsizei indexCount_ = 0;

	//xyz
	std::vector<float> cpuVerts_;
	std::vector<unsigned> cpuIdx_;
};