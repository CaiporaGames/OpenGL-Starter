#pragma once
#include <vector>
#include <cstdint>
#include <glm/vec3.hpp>
#include "core/aabb.hpp"

namespace core
{
	struct MeshData
	{
		//Interleaved? not required. We keep SoA to make CPU ops simple
		std::vector<float> positions;//xyz xyz...
		std::vector<float> normals;//nx, ny, nz ...
		std::vector<float> uvs; //u v...
		std::vector<uint32_t> indices;//triangles - uint32

		AABB aabbModel{}; //computed from positions

		inline std::size_t vertexCount() const
		{
			return positions.size() / 3;
		}
		inline std::size_t triCount() const
		{
			return indices.size() / 3;
		}
	};
}