//Scene populates; pipeline renders. Keeps draw calls centralized.
#pragma once
#include <vector>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include "core/aabb.hpp"

struct MeshGL;

struct DrawItem
{
	const MeshGL* mesh = nullptr;
	glm::mat4 model{ 1.0f };
	glm::vec3 color{ 1.0f, 1.0f, 1.0f };
	core::AABB worldAABB{};
};

struct DrawList
{
	std::vector<DrawItem> items;
	void clear() { items.clear(); }
	void push(const DrawItem& di) { items.push_back(di); }
};