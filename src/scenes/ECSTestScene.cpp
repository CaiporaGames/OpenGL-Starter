#include "scenes/ECSTestScene.hpp"
#include "app/App.hpp"

bool ECSTestScene::init(int fbw, int fbh)
{
    fbw_ = fbw; fbh_ = fbh;
    uiCam_.setViewport(fbw_, fbh_);

    // Spawn camera entity
    auto& eCam = ecs_.createEntity();
    auto& tCam = eCam.add<ecs::Transform>();
    tCam.position = { 0.0f, 1.6f, 6.0f };
    auto& camC = eCam.add<cam::CameraComponent>();
    camC.setPerspective(glm::radians(60.0f), 0.1f, 1000.0f);
    camC.setViewport(fbw_, fbh_);
    camC.setPrimary(true);
    primaryCam_ = &camC;

    ecs_.start();
    return true;
}

void ECSTestScene::onFramebufferResize(int fbw, int fbh)
{
    fbw_ = fbw; fbh_ = fbh;
    uiCam_.setViewport(fbw_, fbh_);
    if (primaryCam_) primaryCam_->setViewport(fbw_, fbh_);
}

void ECSTestScene::update(const FrameInput& /*in*/, float dt)
{
    ecs_.update(dt);
}

void ECSTestScene::render3D(App* /*app*/)
{
    ecs_.render3D(); // (optional: if you add renderable components)
}

void ECSTestScene::render2D(App* /*app*/, SpriteBatch& /*batch*/) const
{
    // Keep empty for now (you can test SpriteBatch here later).
}

void ECSTestScene::onImGui(App* /*app*/)
{
    ecs_.imgui();
}
