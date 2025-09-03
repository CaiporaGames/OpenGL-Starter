#pragma once
#include "engine/ecs/Component.hpp"
#include "engine/ecs/Transform.hpp"
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>


namespace cam
{
	enum class ProjectionMode {Perspective, Ortho};

	struct CameraComponent : ecs::Component
	{
		//Config
		ProjectionMode mode = ProjectionMode::Perspective;
		//Perspective
		float fovY = glm::radians(60.0f);
		//Ortho (height in world units; width = height * aspect)
		float orthoHeight = 10.0f;
		//Clipping
		float nearZ = 0.1f;
		float farZ = 2000.0f;
		//viewport - framebuffer size
		int vpW = 1;
		int vpH = 1;
		bool primary = true; //this camera drives the scene by default

		//Computed each update
		glm::mat4 view{ 1.0f };
		glm::mat4 proj{ 1.0f };
		glm::mat4 vp{ 1.0f };
		glm::vec3 eye{ 0.0f };

		//API
		const char* typeName() const override { return "CameraComponent"; }

		void setViewport(int w, int h)
		{
			vpW = w > 0 ? w : 1;
			vpH = h > 0 ? h : 1;
		}

		void setPerspective(float fovy, float nz, float fz)
		{
			mode = ProjectionMode::Perspective;
			fovY = fovy;
			nearZ = nz;
			farZ = fz;
		}

		void setOrtho(float height, float nz, float fz)
		{
			mode = ProjectionMode::Ortho;
			orthoHeight = height;
			nearZ = nz;
			farZ = fz;
		}

		void setPrimary(bool p) { primary = p; }

		//Lifecycle
		void onUpdate(float dt) override;
		void onImGui() override;
	};
}