#pragma once
#include "engine/ecs/Component.hpp"
#include <glm/vec3.hpp>
#include "core/aabb.hpp"

//fwd; inlcude only where needed to draw
struct MeshGL;

namespace ecs
{
	struct MeshRenderer : Component
	{
		const MeshGL* mesh = nullptr; //Temp: pointer-based 
		core::AABB modelAABB{}; //mesh-space AABB - for culling
		glm::vec3 color{ 0.25f, 0.6f, 0.85f };
		const char* typeName() const override { return "MeshRenderer"; }
	};
}