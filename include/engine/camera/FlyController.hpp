#pragma once
#include "engine/ecs/Component.hpp"

struct FrameInput;

namespace ecs { struct Transform; }

namespace cam
{
	struct FlyController : ecs::Component
	{
		float mouseSensibility = 0.0025f;//radians per pixel
		float moveSpeed = 4.0f;//m/s
		float boostMult = 3.0f; //shift
		float slowMult = 0.25f; //ctrl
		bool requireRMB = true; //mouse look only when RMP held

		// Feed input each frame from the scene before ecs_.update()
		void feedInput(const FrameInput& in);
		//feed input each frame from the scene before ecs_.update()
		void onStart() override {};
		void onUpdate(float dt) override;
		const char* typeName() const override
		{
			return "FlyController";
		}

	private:
		//cached per-frame
		double dx_ = 0.0;
		double dy_ = 0.0;
		bool rmb_ = false;
		bool shift_ = false;
		bool ctrl_ = false;
		bool w_ = false;
		bool a_ = false;
		bool s_ = false;
		bool d_ = false;
		bool q_ = false;
		bool e_ = false;
		bool space_ = false;
		bool c_ = false;
	};
}