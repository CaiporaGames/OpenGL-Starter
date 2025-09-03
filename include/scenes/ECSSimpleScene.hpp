#pragma once
#include "include/engine/IScene.hpp"
#include "include/engine/ecs/Scene.hpp"
#include "include/engine/ecs/Transform.hpp"
#include "render/MeshGL.hpp"
#include "engine/camera/CameraComponent.hpp"

struct ECSSimpleScene : IScene
{
	ecs::Scene ecs_;
	cam::CameraComponent* primaryCam_ = nullptr;
	int fbW_ = 1;
	int fbH_ = 1;

	//Example components:
	struct MeshRenderer3D : ecs::Component
	{
		MeshGL* mesh = nullptr; //external lifetime or switch to shared_ptr
		glm::mat4 model{ 1.0f };
		const char* typeName() const override
		{
			return "MeshRenderer3D";
		}

		void onRender3D() override;
		void onImGui() override;
	};

	struct Spin : ecs::Component
	{
		float speedY = 1.0f;
		const char* typeName() const override { return "Spin"; }
		void onUpdate(float dt) override;
		void onImGui() override;
	};

	//IScene API:
	void onEnter(App* app) override;
	void onExit(App* app) override;
	void update(App* app, float dt) override;
	void render3D(App* app) override;
	void render2D(App* app) override;
	void onImGui(App* app) override;

	//Resize from App (framebuffer size)
	void onFramebufferResize(int w, int h) override;//add to IScene if not present

	//Preferred new API:
	glm::mat4 activeCameraVP() const override
	{
		return primaryCam_ ? primaryCam_->vp : glm::mat4(1.0f);
	}

	glm::vec3 activeCameraPos() const override
	{
		return primaryCam_ ? primaryCam_->eye : glm::vec3(0);
	}

	//Compat shim if scene->camera().vp()
	struct CameraShim {
		const cam::CameraComponent* c = nullptr;
		glm::mat4 vp() const { return c ? c->vp : glm::mat4(1.0f); }
		glm::mat4 proj() const { return c ? c->proj : glm::mat4(1.0f); }
		glm::mat4 view() const { return c ? c->view : glm::mat4(1.0f); }
		glm::vec3 pos() const { return c ? c->eye : glm::vec3(0); }
	} camShim_;
	CameraShim& camera() override { camShim_.c = primaryCam_; return camShim_; }
};