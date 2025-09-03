#include "scenes/ECSSimpleScene.hpp"
#include "gfx/Shader.hpp"
#include "app/App.hpp"

using MR = ECSSimpleScene::MeshRenderer3D;
using SP = ECSSimpleScene::Spin;

void MR::onRender3D()
{
	auto* t = entity->get<ecs::Transform>();
	const glm::mat4 M = t ? t->localMatrix() : model;
	mesh->draw(M);//assume your MeshGL::draw(M) uses current camera VP from App
}

void MR::onImGui()
{
	//show mesh stats
}

void SP::onUpdate(float dt)
{
	if (auto* t = entity->get<ecs::Transform>())
	{
		t->rotationEuler.y += speedY * dt;
	}
}

void SP::onImGui() {/*tweak speed if desired*/ }

void ECSSimpleScene::onEnter(App* app)
{
	ecs_.start();

	//Example: spawn a spinning cube
	auto& e = ecs_.createEntity();
	e.add<ecs::Transform>();
	auto& mr = e.add<MR>();
	mr.mesh = &app->unitCubeMesh();//provide accessor in App or store elsewhere
	e.add<SP>();//Spin behaviour
}

void ECSSimpleScene::update(App* /*app*/, float dt) { ecs_.update(dt); }
void ECSSimpleScene::render3D(App* /*app*/) { ecs_.render3D(); }
void ECSSimpleScene::render2D(App* /*app*/) { ecs_.render2D(); }
void ECSSimpleScene::onImGui(App* /*app*/) { ecs_.imgui(); }