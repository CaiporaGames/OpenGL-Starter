#pragma once
#include <vector>
#include <algorithm>
#include <functional>
#include <cstddef>
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>
#include "core/aabb.hpp"
#include "core/ray.hpp"
#include "core/raycast.hpp"

namespace core
{
	struct BVHNode 
	{
		AABB  box;
		int   left = -1;   // child index in 'nodes' (for internal)
		int   right = -1;   // child index in 'nodes' (for internal)
		int   first = 0;    // first triangle index into 'indices' (for leaf)
		int   count = 0;    // number of triangles in leaf; 0 => internal
		bool isLeaf() const noexcept { return count > 0; }
	};

	struct BVH
	{
		std::vector<BVHNode> nodes;
		std::vector<unsigned> indices;//reordered triangle indices in triplets
	};

	// Helpers
	inline AABB merge(const AABB& a, const AABB& b)
	{
		AABB r;
		r.min =
		{
			std::min(a.min.x, b.min.x), std::min(a.min.y, b.min.y), std::min(a.min.z, b.min.z)
		};
		r.max =
		{
			std::max(a.max.x, b.max.x), std::max(a.max.y, b.max.y), std::max(a.max.z, b.max.z)
		};
		return r;
	}

	inline AABB triAABB(const float* P, unsigned i0, unsigned i1, unsigned i2)
	{
		const glm::vec3 a
		{
			P[3 * i0 + 0], P[3 * i0 + 1], P[3 * i0 + 2]
		};

		const glm::vec3 b
		{
			P[3 * i1 + 0], P[3 * i1 + 1], P[3 * i1 + 2]
		};

		const glm::vec3 c
		{
			P[3 * i2 + 0], P[3 * i2 + 1], P[3 * i2 + 2]
		};

		AABB box;
		box.min = box.max = a;

		box.min.x = std::min({ a.x, b.x, c.x });
		box.min.y = std::min({ a.y, b.y, c.y });
		box.min.z = std::min({ a.z, b.z, c.z });

		box.max.x = std::max({ a.x, b.x, c.x });
		box.max.y = std::max({ a.y, b.y, c.y });
		box.max.z = std::max({ a.z, b.z, c.z });

		return box;
	}

	inline glm::vec3 triCentroid(const float* P, unsigned i0, unsigned i1, unsigned i2)
	{
		const glm::vec3 a
		{
			P[3 * i0 + 0], P[3 * i0 + 1], P[3 * i0 + 2]
		};
		const glm::vec3 b
		{
			P[3 * i1 + 0], P[3 * i1 + 1], P[3 * i1 + 2]
		};
		const glm::vec3 c
		{
			P[3 * i2 + 0], P[3 * i2 + 1], P[3 * i2 + 2]
		};

		return (a + b + c) * (1.0f/3.0f);
	}

	//Ray vs AABB slab. Returns true if hit with t in [0, tMax]; outputs tNear.
	inline bool rayAABB(const Ray& r, const AABB& b, float& tNear, float tMax=1e30f)
	{
		float tmin = 0.0f;
		float tmax = tMax;

		auto slab = [&](float o, float d, float mn, float mx) -> bool
		{
			const float eps = 1e-8;

			if (std::abs(d) < eps)
			{
				return (o >= mn && o <= mx);
			}

			float ood = 1.0f / d;
			float t1 = (mn - o) * ood;
			float t2 = (mx - o) * ood;

			if (t1 > t2)
			{
				std::swap(t1, t2);
			}

			tmin = std::max(tmin, t1);
			tmax = std::min(tmax, t2);

			return tmin <= tmax;
		};

		if (!slab(r.origin.x, r.dir.x, b.min.x, b.max.x)) return false;
		if (!slab(r.origin.y, r.dir.y, b.min.y, b.max.y)) return false;
		if (!slab(r.origin.z, r.dir.z, b.min.z, b.max.z)) return false;

		tNear = tmin;

		return true;
	}

	//Build median split on largest centroid axis
	namespace detail
	{
		struct Prim
		{
			//Triangle indices
			unsigned i0;
			unsigned i1;
			unsigned i2;

			AABB box;
			glm::vec3 c; //centroid
		};

		inline int newNode(std::vector<BVHNode>& nodes)
		{
			nodes.push_back(BVHNode{}); 

			return int(nodes.size()-1);
		}
	}

	inline BVH buildBVH(const float* positions, const unsigned* indices, 
		std::size_t triCount, int leafMaxTris = 4)
	{
		using namespace detail;
		BVH bvh;

		bvh.indices.resize(triCount*3);
		std::vector<Prim> prims(triCount);

		//Init prims
		for (std::size_t t = 0; t < triCount; ++t)
		{
			unsigned i0 = indices[3 * t + 0];
			unsigned i1 = indices[3 * t + 1];
			unsigned i2 = indices[3 * t + 2];

			prims[t] = Prim
			{
				i0, i1, i2, triAABB(positions, i0, i1, i2), triCentroid(positions, i0, i1, i2)
			};
		}

		//working permutation of triangles triplets (store as triplets in bvh.indices)
		std::vector<unsigned> order(triCount);

		for (std::size_t i = 0; i < triCount; ++i)
		{
			order[i] = (unsigned)i;
		}

		// Build
		bvh.nodes.reserve(std::max<std::size_t>(1, triCount * 2));
		std::function<int(int, int)> build = [&](int begin, int end)->int {
			int nodeIdx = newNode(bvh.nodes);
			// Compute node bounds
			AABB bounds = prims[order[begin]].box;
			AABB cb; cb.min = cb.max = prims[order[begin]].c;
			for (int i = begin + 1; i < end; ++i) {
				bounds = merge(bounds, prims[order[i]].box);
				const glm::vec3 c = prims[order[i]].c;
				cb.min.x = std::min(cb.min.x, c.x); cb.max.x = std::max(cb.max.x, c.x);
				cb.min.y = std::min(cb.min.y, c.y); cb.max.y = std::max(cb.max.y, c.y);
				cb.min.z = std::min(cb.min.z, c.z); cb.max.z = std::max(cb.max.z, c.z);
			}
			bvh.nodes[nodeIdx].box = bounds;

			const int count = end - begin;
			if (count <= leafMaxTris) {
				// Write triangles into final index buffer (triplets kept)
				const int first = (int)((begin) * 3);
				if ((int)bvh.indices.size() < (first + count * 3))
					bvh.indices.resize(first + count * 3);
				for (int i = 0; i < count; i++) {
					const Prim& p = prims[order[begin + i]];
					bvh.indices[first + 3 * i + 0] = p.i0;
					bvh.indices[first + 3 * i + 1] = p.i1;
					bvh.indices[first + 3 * i + 2] = p.i2;
				}
				bvh.nodes[nodeIdx].first = first;
				bvh.nodes[nodeIdx].count = count;
				return nodeIdx;
			}

			// Choose split axis = largest extent of centroid bounds
			glm::vec3 ext = cb.max - cb.min;
			int axis = 0;
			if (ext.y > ext.x && ext.y >= ext.z) axis = 1;
			else if (ext.z > ext.x && ext.z >= ext.y) axis = 2;

			const float mid = 0.5f * ((&cb.min.x)[axis] + (&cb.max.x)[axis]);

			// Partition by centroid (median split). Guard against degenerate extents.
			int midIdx = begin + count / 2;
			auto itBegin = order.begin() + begin, itMid = order.begin() + midIdx, itEnd = order.begin() + end;
			std::nth_element(itBegin, itMid, itEnd, [&](unsigned a, unsigned b) {
				return (&prims[a].c.x)[axis] < (&prims[b].c.x)[axis];
				});

			// If all on one side (degenerate), force split
			if (midIdx == begin || midIdx == end) midIdx = begin + count / 2;

			int L = build(begin, midIdx);
			int R = build(midIdx, end);
			bvh.nodes[nodeIdx].left = L;
			bvh.nodes[nodeIdx].right = R;
			return nodeIdx;
			};

		build(0, (int)triCount);
		return bvh;
	}


	//Raycast using BVH 
	struct BVHStats
	{
		int nodesVisited = 0;
		int leafTests = 0;
		int triTests = 0;
	};

	inline bool raycastBVH(const Ray& r, const float* positions, const BVH& bvh,
		RayHit& out, BVHStats* stats = nullptr)
	{
		if (bvh.nodes.empty()) return false;

		float bestT = 1e30f;
		int bestTriFirst = -1;//index into bvh.indices - triplet base
		float bu = 0.0;
		float bv = 0.0;

		//manual stack
		int stack[64];
		int sp = 0;
		stack[sp++] = 0;

		while (sp > 0)
		{
			const int ni = stack[--sp];
			const BVHNode& n = bvh.nodes[ni];

			if (stats) ++stats->nodesVisited;

			float tNear;

			if (!rayAABB(r, n.box, tNear, bestT)) continue;

			if (n.isLeaf())
			{
				if (stats) ++stats->leafTests;

				for (int i = 0; i < n.count; i++)
				{
					const unsigned i0 = bvh.indices[n.first + 3 * i + 0];
					const unsigned i1 = bvh.indices[n.first + 3 * i + 1];
					const unsigned i2 = bvh.indices[n.first + 3 * i + 2];

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

					if (stats) ++stats->triTests;

					if (rayTriangle(r, v0, v1, v2, t, u, v) && t < bestT)
					{
						bestT = t;
						bestTriFirst = n.first + 3 * i;
						bu = u;
						bv = v;
					}
				}
			}
			else
			{
				//visit nearer child first
				float tL = 1e30f;
				float tR = 1e30f;

				bool hL = (n.left >= 0) && rayAABB(r, bvh.nodes[n.left].box, tL, bestT);
				bool hR = (n.right >= 0) && rayAABB(r, bvh.nodes[n.right].box, tR, bestT);
				
				if (hL && hR)
				{
					//push farther first so nearer is processed next
					if (tL > tR)
					{
						stack[sp++] = n.left;
						stack[sp++] = n.right;
					}
					else
					{
						stack[sp++] = n.right;
						stack[sp++] = n.left;
					}
				}
				else if (hL) stack[sp++] = n.left;
				else if (hR) stack[sp++] = n.right;
			}
		}

		if (bestTriFirst >= 0)
		{
			out.t = bestT;
			out.u = bu;
			out.v = bv;
			out.triIndex = bestTriFirst / 3; //index into reordered list

			return true;
		}

		return false;
	}
}