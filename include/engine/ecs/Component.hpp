#pragma once
#include <string>

namespace ecs
{
	struct Entity;
	struct Scene;

	struct Component
	{
		Entity* entity = nullptr; //set by Entity on attach
		Scene* scene = nullptr; //set by Scene on spawn

		virtual ~Component() = default;
		virtual const char* typeName() const = 0;

		//Lifecycle 
		virtual void onAttach() {}
		virtual void onDetach() {}
		virtual void onStart() {} //first frame after added/spawned
		virtual void onUpdate(float dt) {}
		virtual void onRender3D() {} //draw into 3D pass - if needed
		virtual void onRender2D() {} //SpriteBacth/UI - if needed
		virtual void onImGui() {} //inpector/debug UI
	};
}