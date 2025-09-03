#include "engine/camera/CameraComponent.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include "engine/ecs/Entity.hpp"      
#include "engine/ecs/Transform.hpp" 

namespace cam
{
	static inline void computeViewFromTransform(const ecs::Transform* t, glm::mat4& V, glm::vec3& eye)
	{
		glm::vec3 pos = t ? t->position : glm::vec3(0);
		glm::vec3 eul = t ? t->rotationEuler : glm::vec3(0);

		//build a rotation basis from yaw-pitch-roll (x,y,z)
		glm::mat4 R = glm::yawPitchRoll(eul.x, eul.y, eul.z);
		glm::vec3 forward = glm::normalize(glm::vec3(R * glm::vec4(0, 0, -1, 0)));
		glm::vec3 up = glm::normalize(glm::vec3(R * glm::vec4(0, 1, 0, 0)));
		V = glm::lookAt(pos, pos + forward, up);
		eye = pos;
	}

	static inline glm::mat4 computeProj(ProjectionMode mode, float fovY, float orthoHeight, float nearZ, float farZ, int w, int h)
	{
		const float aspect = h > 0 ? (float)w / (float)h : 1.0f;

		if (mode == ProjectionMode::Perspective)
		{
			return glm::perspective(fovY, aspect, nearZ, farZ);
		}
		else
		{
			const float halfH = 0.5f * orthoHeight;
			const float halfW = aspect * halfH;

			return glm::ortho(-halfW, halfW, -halfH, halfH, nearZ, farZ);
		}
	}

	void CameraComponent::onUpdate(float dt)
	{
		//view from transform
		auto* t = entity ? entity->get<ecs::Transform>() : nullptr;
		computeViewFromTransform(t, view, eye);

		//projection form params + viewport
		proj = computeProj(mode, fovY, orthoHeight, nearZ, farZ, vpW, vpH);
		vp = proj * view;
	}

	void CameraComponent::onImGui()
	{
		// Optional: quick inspector (safe if ImGui not compiled)
#ifdef IMGUI_VERSION
		if (ImGui::TreeNode("Camera")) {
			int pm = (mode == ProjectionMode::Perspective) ? 0 : 1;
			if (ImGui::RadioButton("Perspective", pm == 0)) mode = ProjectionMode::Perspective, pm = 0;
			ImGui::SameLine();
			if (ImGui::RadioButton("Ortho", pm == 1)) mode = ProjectionMode::Ortho, pm = 1;

			if (mode == ProjectionMode::Perspective) {
				ImGui::SliderAngle("FOV Y", &fovY, 20.0f, 120.0f);
			}
			else {
				ImGui::SliderFloat("Ortho Height", &orthoHeight, 1.0f, 200.0f);
			}
			ImGui::SliderFloat("Near", &nearZ, 0.01f, 5.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
			ImGui::SliderFloat("Far", &farZ, 10.0f, 10000.0f, "%.0f", ImGuiSliderFlags_Logarithmic);
			ImGui::Text("Viewport: %dx%d", vpW, vpH);
			ImGui::Checkbox("Primary", &primary);
			ImGui::TreePop();
		}
#endif
	}
}