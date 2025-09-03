#pragma once
#include <vector>
#include <memory>
#include "engine/ecs/Entity.hpp"

namespace ecs
{
	struct Scene
	{
		Entity& createEntity()
		{
			auto e = std::make_unique<Entity>();
			Entity* raw = e.get();

			//Set scene pointer into all current comps when added
			entities_.push_back(std::move(e));
			dirtyStart_ = true;
			return *raw;
		}

		void start();//call on scene enter
		void update(float dt);
		void render3D();
		void render2D();
		void imgui();

	private:
		std::vector<std::unique_ptr<Entity>> entities_;
		bool dirtyStart_ = false;
	};

}
