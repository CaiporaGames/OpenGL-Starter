#pragma once
#include "engine/IScene.hpp"
#include "engine/ecs/Scene.hpp"
#include "engine/ecs/Transform.hpp"
#include "engine/camera/CameraComponent.hpp"

struct ECSTestScene : IScene 
{
    ecs::Scene ecs_;
    cam::CameraComponent* primaryCam_ = nullptr;
    OrthoCamera2D uiCam_;
    int fbw_ = 1, fbh_ = 1;

    bool init(int fbw, int fbh) override;
    void onFramebufferResize(int fbw, int fbh) override;
    void resize(int fbw, int fbh) override { onFramebufferResize(fbw, fbh); }
    void update(const FrameInput& in, float dt) override;
    void render3D(App* app) override;       // optional; we draw in pipeline already
    void render2D(App* app, SpriteBatch& batch) const override;
    void onImGui(App* app) override;

    glm::mat4 activeCameraVP() const override { return primaryCam_ ? primaryCam_->vp : glm::mat4(1.0f); }
    glm::vec3 activeCameraPos() const override { return primaryCam_ ? primaryCam_->eye : glm::vec3(0); }

    OrthoCamera2D& uiCamera() override { return uiCam_; }
    const OrthoCamera2D& uiCamera() const override { return uiCam_; }
};
