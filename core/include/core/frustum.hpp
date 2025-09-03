#pragma once
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>
#include "core/aabb.hpp"

namespace core
{
	//plane: dot(n, x) + d = 0 with |n| = 1
	struct Plane
	{
		glm::vec3 n; 
		float d;
	};

	struct Frustum
	{
		//order: 0=L, 1=R, 2=B, 3=T, 4=N, 5=F
		Plane p[6];
	};

	inline Frustum extractFrustum(const glm::mat4& VP)
	{
		Frustum fr{};

		const glm::mat4 m = glm::transpose(VP);

		const glm::vec4 eq[6] =
		{
			m[3] + m[0], // Left
			m[3] - m[0], // Right
			m[3] + m[1], // Bottom
			m[3] - m[1], // Top
			m[3] + m[2], // Near
			m[3] - m[2], // Far
		};

		for (int i = 0; i < 6; i++)
		{
			glm::vec3 n(eq[i].x, eq[i].y, eq[i].z);
			float d = eq[i].w;
			const float len = glm::length(n);

			if (len > 0.0f)
			{
				n /= len;
				d /= len;
			}

			fr.p[i] = Plane{ n, d };
		}

		return fr;
	}

	//Fast AABB vs plane test using "positive vertex". Returns true if ouside.
	inline bool aabbOutsidePlane(const Plane& pl, const AABB& b)
	{
		const glm::vec3 p =
		{
			(pl.n.x >= 0.0f) ? b.max.x : b.min.x,
			(pl.n.y >= 0.0f) ? b.max.y : b.min.y,
			(pl.n.z >= 0.0f) ? b.max.z : b.min.z
		};

		return glm::dot(pl.n, p) + pl.d < 0.0f;
	}

	//Returns true if the AABB is outside the frustum = culled
	inline bool aabbOutsideFrustum(const Frustum& fr, const AABB& b)
	{
		//if outside any place -> culled
		for (int i = 0; i < 6; i++)
			if (aabbOutsidePlane(fr.p[i], b)) return true;
		
		return false; //inside or intersecting
	}
}

