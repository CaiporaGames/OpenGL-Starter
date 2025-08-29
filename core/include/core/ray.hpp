#pragma once
#include <glm/vec3.hpp>

namespace core
{
	struct Ray
	{
		glm::vec3 origin;
		glm::vec3 dir; //must be normalized
	};

	struct RayHit
	{
		float t = 0.0f; //distance along ray
		float u = 0.0f;
		float v = 0.0f;//baricentrics
		int triIndex = -1;//triangle id (0...triCount-1)
	};
}