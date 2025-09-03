#pragma once
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "engine/ecs/Component.hpp"
#include <imgui.h>

namespace ecs
{
	struct Transform : Component
	{
		glm::vec3 position{ 0 };
		glm::vec3 rotationEuler{ 0 };//radius (pithc=x, yaw=y, roll=z)
		glm::vec3 scale{1, 1, 1};

		glm::mat4 localMatrix() const
		{
			glm::mat4 m(1.0f);
			m = glm::translate(m, position);
			m = glm::rotate(m, rotationEuler.x, glm::vec3(1, 0, 0));
			m = glm::rotate(m, rotationEuler.y, glm::vec3(0, 1, 0));
			m = glm::rotate(m, rotationEuler.z, glm::vec3(0, 0, 1));

			m = glm::scale(m, scale);

			return m;		
		}

		const char* typeName() const override
		{
			return "Transform";
		}

		void onImGui() override
		{
			// Example ImGui UI for Transform
			ImGui::DragFloat3("Position", &position.x, 0.1f);
			ImGui::DragFloat3("Rotation", &rotationEuler.x, 0.1f);
			ImGui::DragFloat3("Scale", &scale.x, 0.1f);
		}
	};
}