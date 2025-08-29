#include "render/BoxWire3D.hpp"

bool BoxWire3D::init()
{
	glGenVertexArrays(1, &vao_);
	glGenBuffers(1, &vbo_);
	glBindVertexArray(vao_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*24, nullptr, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), NULL);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	ready_ = false;

	return true;
}

void BoxWire3D::set(const glm::vec3& mn, const glm::vec3& mx)
{
	const glm::vec3 p[8] =
	{
		{mn.x,mn.y,mn.z},{mx.x,mn.y,mn.z},{mn.x,mx.y,mn.z},{mx.x,mx.y,mn.z},
		{mn.x,mn.y,mx.z},{mx.x,mn.y,mx.z},{mn.x,mx.y,mx.z},{mx.x,mx.y,mx.z},
	};

	const int e[12][2] =
	{
		{0,1},{1,3},{3,2},{2,0}, // bottom rectangle
		{4,5},{5,7},{7,6},{6,4}, // top rectangle
		{0,4},{1,5},{2,6},{3,7}  // verticals
	};

	float data[24 * 3];

	for (int i = 0; i < 12; i++)
	{
		glm::vec3 a = p[e[i][0]];
		glm::vec3 b = p[e[i][1]];

		data[i * 6 + 0] = a.x;
		data[i * 6 + 1] = a.y;
		data[i * 6 + 2] = a.z;

		data[i * 6 + 3] = b.x;
		data[i * 6 + 4] = b.y;
		data[i * 6 + 5] = b.z;
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(data), data);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	ready_ = true;
}

void BoxWire3D::draw() const
{
	if (!ready_) return;
	glBindVertexArray(vao_);
	glDrawArrays(GL_LINES, 0, 24);
	glBindVertexArray(0);
}

void BoxWire3D::destroy()
{
	if (vbo_) glDeleteBuffers(1, &vbo_), vbo_ = 0;
	if (vao_) glDeleteVertexArrays(1, &vao_), vao_ = 0;

	ready_ = false;
}