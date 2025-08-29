#pragma once
#include <glm/vec3.hpp>
#include <cstddef>
#include <limits>
#include <glm/mat4x4.hpp>
#include <algorithm>

namespace core
{
	struct AABB { glm::vec3 min, max; };

	inline AABB computeAABB(const float* xyz, std::size_t count)
	{
		AABB paralelogram;
		paralelogram.min =
		{
			+std::numeric_limits<float>::infinity(),
			+std::numeric_limits<float>::infinity(),
			+std::numeric_limits<float>::infinity()
		};
		paralelogram.max =
		{
			-std::numeric_limits<float>::infinity(),
			-std::numeric_limits<float>::infinity(),
			-std::numeric_limits<float>::infinity()
		};

		for (std::size_t i = 0; i < count; ++i)
		{
			const float x = xyz[3 * i + 0];
			const float y = xyz[3 * i + 1];
			const float z = xyz[3 * i + 2];

			if (x < paralelogram.min.x) paralelogram.min.x = x;
			if (x > paralelogram.max.x) paralelogram.max.x = x;
			if (y < paralelogram.min.y) paralelogram.min.y = y;
			if (y > paralelogram.max.y) paralelogram.max.y = y;
			if (z < paralelogram.min.z) paralelogram.min.z = z;
			if (z > paralelogram.max.z) paralelogram.max.z = z;
		}
		return paralelogram;
	}

	inline AABB transformAABB(const AABB& b, const glm::mat4& M)
	{
		const glm::vec3 c[8] =
		{
			{b.min.x,b.min.y,b.min.z},{b.max.x,b.min.y,b.min.z},
			{b.min.x,b.max.y,b.min.z},{b.max.x,b.max.y,b.min.z},
			{b.min.x,b.min.y,b.max.z},{b.max.x,b.min.y,b.max.z},
			{b.min.x,b.max.y,b.max.z},{b.max.x,b.max.y,b.max.z},
		};
		AABB out;
		out.min = { +INFINITY, +INFINITY, +INFINITY };
		out.max = { -INFINITY, -INFINITY, -INFINITY };

		for (auto& p : c)
		{
			glm::vec3 w = glm::vec3(M * glm::vec4(p, 1.0f));

			out.min.x = std::min(out.min.x, w.x);
			out.min.y = std::min(out.min.y, w.y);
			out.min.z = std::min(out.min.z, w.z);

			out.max.x = std::max(out.max.x, w.x);
			out.max.y = std::max(out.max.y, w.y);
			out.max.z = std::max(out.max.z, w.z);
		}
		return out;
	}
}