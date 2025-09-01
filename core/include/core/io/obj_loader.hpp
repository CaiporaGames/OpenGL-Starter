#pragma once
#include <string>
#include "core/mesh_data.hpp"

namespace core::io
{
	//load triangles from .obj - single or multi-shapes. Returns true on success
	// if has normals/uvs, they're filled; otherwise vectors stay empty.
	//Triangulates n-gons as provided by tinyobjloader
	//baseDir is optional; pass "" to auto-derive from path
	bool loadOBJ(const std::string& path, MeshData& out, std::string* outErr = nullptr, const std::string& baseDir = "");
}