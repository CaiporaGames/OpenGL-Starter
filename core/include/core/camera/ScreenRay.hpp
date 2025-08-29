#pragma once
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "core/ray.hpp"

namespace core
{
	//build a world-space ray from screen GLFW pixels.
	//fbw/fbh = frameBuffer size
	inline Ray screenRayFromInvVP(const glm::mat4& invVP, double sx, double sy, int fbw, int fbh)
	{
		const float x = float((2.0 * sx) / double(fbw) - 1.0);
		const float y = float(1.0 - (2.0*sy)/ double(fbh));
		const glm::vec4 nearNDC(x, y, -1.0f, 1.0f);
		const glm::vec4 farNDC(x, y, +1.0f, 1.0f);

		glm::vec4 nearW = invVP * nearNDC;
		glm::vec4 farW = invVP * farNDC;
		nearW /= nearW.w;
		farW /= farW.w;

		core::Ray ray;
		ray.origin = glm::vec3(nearW);
		ray.dir = glm::normalize(glm::vec3(farW-nearW));

		return ray;
	}
}