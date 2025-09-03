#pragma once
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>
#include <cstddef>
#include "core/ray.hpp"

namespace core
{
	//// Möller–Trumbore. Returns true on hit with t>0.
	inline bool rayTriangle(const Ray& r, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3 v2, float& t, float& u, float& v)
	{
		const glm::vec3 e1 = v1 - v0;
		const glm::vec3 e2 = v2 - v0;
		const glm::vec3 p = glm::cross(r.dir, e2);
		const float det = glm::dot(e1, p);
		const float eps = 1e-8f;
		
		if (det > -eps && det < eps) return false; //parallel

		const float invDet = 1.0f / det;
		const glm::vec3 tvec = r.origin - v0;
		u = glm::dot(tvec, p) * invDet;

		if (u < 0.0f || u > 1.0f) return false;

		const glm::vec3 qvec = glm::cross(tvec, e1);
		v = glm::dot(r.dir, qvec) * invDet;

		if (v < 0.0f || u + v > 1.0f) return false;

		t = glm::dot(e2, qvec) * invDet;

		return t > eps;
	}

	// positions = tightly-packed xyz float array
	// indices   = uint32/uint (triplets form triangles)
	// triCount  = number of triangles (indices.size()/3)
	//
	// Returns true and fills 'out' with nearest hit in FRONT of ray.
	inline bool raycastMesh(const Ray& r, const float* positions, const unsigned* indices, std::size_t triCount, RayHit& out)
	{
		bool hit = false;
		float bestT = 1e30f;
		int bestTri = -1;
		float bu = 0.0f;
		float bv = 0.0f;

		for (std::size_t i = 0; i < triCount; ++i)
		{
			const unsigned i0 = indices[3 * i + 0];
			const unsigned i1 = indices[3 * i + 1];
			const unsigned i2 = indices[3 * i + 2];

			const glm::vec3 v0
			{
				positions[3 * i0 + 0], positions[3 * i0 + 1], positions[3 * i0 + 2]
			};
			const glm::vec3 v1
			{
				positions[3 * i1 + 0], positions[3 * i1 + 1], positions[3 * i1 + 2]
			};
			const glm::vec3 v2
			{
				positions[3 * i2 + 0], positions[3 * i2 + 1], positions[3 * i2 + 2]
			};

			float t;
			float u;
			float v;

			if (rayTriangle(r, v0, v1, v2, t, u, v) && t < bestT)
			{
				bestT = t;
				bestTri = (int)i;
				bu = u;
				bv = v;
				hit = true;
			}
		}

		if (hit)
		{
			out.t = bestT;
			out.u = bu;
			out.v = bv;
			out.triIndex = bestTri;
		}

		return hit;
	}
}