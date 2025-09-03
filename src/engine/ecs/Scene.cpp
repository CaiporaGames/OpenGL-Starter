#include "engine/ecs/Scene.hpp"

namespace ecs
{
	void Entity::onStart()
	{
		for (auto& c : pendingStart_)
		{
			c->onStart();
			pendingStart_.clear();
		}
	}

	void Entity::onUpdate(float dt)
	{
		for (auto& c : components_)
		{
			c->onUpdate(dt);
		}
	}

	void Entity::onRender3D()
	{
		for (auto& c : components_)
		{
			c->onRender3D();
		}
	}
	void Entity::onRender2D()
	{
		for (auto& c : components_)
		{
			c->onRender2D();
		}
	}
	void Entity::onImGui()
	{
		for (auto& c : components_)
		{
			c->onImGui();
		}
	}

	void Scene::start()
	{
		dirtyStart_ = true;
	}

	void Scene::update(float dt)
	{
		if (dirtyStart_)
		{
			for (auto& e : entities_)
			{
				e->onStart();
			}
			dirtyStart_ = false;
		}

		for (auto& e : entities_)
		{
			if (e->active) e->onUpdate(dt);
		}
	}

	void Scene::render3D()
	{
		for (auto& e : entities_)
		{
			if (e->active) e->onRender3D();
		}
	}

	void Scene::render2D()
	{
		for (auto& e : entities_)
		{
			if (e->active) e->onRender2D();
		}
	}

	void Scene::imgui()
	{
		for (auto& e : entities_)
		{
			if (e->active) e->onImGui();
		}
	}
}