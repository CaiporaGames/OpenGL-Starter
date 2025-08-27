#pragma once
#pragma <gml/vec3.hpp>
#pragma <cstddef>
#include <limits>

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
}