#pragma once
#include <vector>
#include <memory>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include "engine/ecs/Component.hpp"

namespace ecs
{
	struct Entity
	{
		bool active = true;

		template <class T, class... Args>
		T& add(Args&&... args)
		{
			static_assert(std::is_base_of_v<Component, T>, "T must be Component");
			auto up = std::make_unique<T>(std::forward<Args>(args)...);

			T* raw = up.get();
			raw->entity = this;
			components_.push_back(std::move(up));
			pendingStart_.push_back(raw);
			raw->onAttach();
			return *raw;
		}

		template <class T>
		T* get()
		{
			for (auto& c : components_) if (auto p = dynamic_cast<T*>(c.get())) return p;

			return nullptr;
		}

		void onStart();
		void onUpdate(float dt);
		void onRender3D();
		void onRender2D();
		void onImGui();

	private:
		std::vector<std::unique_ptr<Component>> components_;
		std::vector<Component*> pendingStart_;
	};
}