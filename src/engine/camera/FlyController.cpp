#define GLM_ENABLE_EXPERIMENTAL
#include "engine/camera/FlyController.hpp"
#include "engine/ecs/Entity.hpp"
#include "engine/ecs/Transform.hpp"
#include "engine/IScene.hpp"
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <algorithm>

using glm::vec3; 
using glm::mat4;

namespace cam
{
	void FlyController::feedInput(const FrameInput& in)
	{
		dx_ = in.mouseDeltaX;
		dy_ = in.mouseDeltaY;
		rmb_ = in.rmbDown;
		shift_ = in.lshiftDown;
		ctrl_ = in.lctrlDown;

		//prefer explicit WASD if present; fallback to old W/S aliases
		w_ = in.keyW;
		s_ = in.keyS;
		a_ = in.keyA;
		d_ = in.keyD;
		q_ = in.keyQ;
		e_ = in.keyE;
		space_ = in.keySpace;
		c_ = in.keyC;
	}

	void FlyController::onUpdate(float dt)
	{
		auto* t = entity->get<ecs::Transform>();

		if (!t) return;

		//mouse look - convention: rotationEuler = {yaw(x), pitch(y), roll(z)})
		const bool looking = !requireRMB || rmb_;
		
		if (looking)
		{
			t->rotationEuler.x -= static_cast<float>(dx_) * mouseSensibility;//yaw
			t->rotationEuler.y -= static_cast<float>(dy_) * mouseSensibility;//pitch

			const float lim = glm::radians(89.0f);
			t->rotationEuler.y = std::clamp<float>(t->rotationEuler.y, -lim, lim);
		}

		//Basis from yaw/pitch - matches CameraComponent
		mat4 R = glm::yawPitchRoll(t->rotationEuler.x, t->rotationEuler.y, t->rotationEuler.z);
		vec3 f = glm::normalize(vec3(R * glm::vec4(0, 0, -1, 0)));
		vec3 r = glm::normalize(vec3(R * glm::vec4(1, 0, 0, 0)));
		vec3 u = vec3(0, 1, 0);

		//FPS-style: horizontal movement on XZ, vertical with Q/E/Space/C
		vec3 move(0);
		vec3 f_xz = glm::normalize(vec3(f.x, 0, f.z));

		if (w_) move += f_xz;
		if (s_) move -= f_xz;
		if (a_) move -= r;
		if (d_) move += r;

		if (q_ || c_) move += vec3(0, -1, 0);
		if (e_ || space_) move += vec3(0, 1, 0);

		if (glm::dot(move, move) > 0.0f) move = glm::normalize(move);

		float spd = moveSpeed;

		if (shift_) spd *= boostMult;
		if (ctrl_) spd *= slowMult;

		t->position += move * spd * dt;
	}	

}