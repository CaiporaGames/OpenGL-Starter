#include "core/io/obj_loader.hpp"
#include <tiny_obj_loader.h>
#include <filesystem>
#include <limits>
#include "core/aabb.hpp"

namespace core::io
{
	static inline std::string parentDir(const std::string& p)
	{
		try
		{
			return std::filesystem::path(p).parent_path().string();
		}
		catch (...)
		{
			return "";
		}
	}

	bool loadOBJ(const std::string& path, MeshData& out, std::string* outErr, const std::string& baseDirIn)
	{
		out = MeshData{};

		tinyobj::ObjReaderConfig cfg;
		cfg.mtl_search_path = baseDirIn.empty() ? parentDir(path) : baseDirIn;
		cfg.triangulate = true;//ensure triangles

		tinyobj::ObjReader reader;

		if (!reader.ParseFromFile(path, cfg))
		{
			if (outErr)*outErr = reader.Error();

			return false;
		}
		if (!reader.Warning().empty() && outErr)
		{
			*outErr = reader.Warning();//non-fatal
		}

		const auto& attrib = reader.GetAttrib();
		const auto& shapes = reader.GetShapes();

		//Collect unique vertices via index triplets; but for picking we only need positions + trianles
		out.positions.resize(attrib.vertices.size());

		if (!attrib.vertices.empty())
		{
			std::copy(attrib.vertices.begin(), attrib.vertices.end(), out.positions.begin());
		}
		//Normals/uvs 
		out.normals.resize(attrib.normals.size());

		if (!attrib.normals.empty())
		{
			std::copy(attrib.normals.begin(), attrib.normals.end(), out.normals.begin());
		}

		out.uvs.resize(attrib.texcoords.size());
		
		if (!attrib.texcoords.empty())
		{
			std::copy(attrib.texcoords.begin(), attrib.texcoords.end(), out.uvs.begin());
		}

		//Build a unified triangle index buffer (indices reference - positions - array directly)
		//tinyobj indices are per-shape and per-verted; for positions they are .vertex_index
		size_t triCount = 0;

		for (const auto& s : shapes)
		{
			triCount += s.mesh.indices.size() / 3;
		}
		out.indices.reserve(triCount * 3);

		for (const auto& s : shapes)
		{
			const auto& idx = s.mesh.indices;
			for (size_t i = 0; i + 2 < idx.size(); i += 3)
			{
				out.indices.push_back(static_cast<uint32_t>(idx[i + 0].vertex_index));
				out.indices.push_back(static_cast<uint32_t>(idx[i + 1].vertex_index));
				out.indices.push_back(static_cast<uint32_t>(idx[i + 2].vertex_index));
			}
		}

		//AABB model/local
		if (!out.positions.empty())
		{
			out.aabbModel = core::computeAABB(out.positions.data(), out.vertexCount());
		}
		return true;
	}
}