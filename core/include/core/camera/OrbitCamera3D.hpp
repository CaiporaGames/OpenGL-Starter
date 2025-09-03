#pragma once
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

namespace core
{
	class OrbitCamera3D
	{
		public:
			void setViewport(int fbw, int fbh)
			{
				fbw_ = std::max(1, fbw);
				fbh_ = std::max(1, fbh);
			}

			void setLens(float fovY_deg, float zNear, float zFar)
			{
				fovY_ = fovY_deg;
				zn_ = zNear;
				zf_ = zFar;
			}

			void setTarget(const glm::vec3& t)
			{
				target_ = t;
			}

			void setDistance(float d)
			{
				dist_ = std::clamp(d, 0.01f, 1e6f);
			}

			void setAngles(float yaw_deg, float pitch_deg)
			{
				yaw_ = yaw_deg;
				pitch_ = clampPitch(pitch_deg);
			}

			//mouse deltas (pixels) -> yaw/pitch (deg) with sensitivity
			void tumble(float dxPixels, float dyPixels, float sensDegPerPx = 0.2f)
			{
				yaw_ += dxPixels * sensDegPerPx;
				pitch_ = clampPitch(pitch_+dyPixels * sensDegPerPx);
			}
			//+wheel => zoom in
			void dolly(float wheelY, float sens = 1.2f)
			{
				if (wheelY == 0.0f) return;
				float f = (wheelY > 0.0f) ? 1.0f / sens : sens;
				setDistance(f * dist_);
			}

			glm::vec3 position() const
			{
				static constexpr float kPi = 3.1415926535f;
				float yaw = yaw_ * (kPi / 180.f);
				float pit = pitch_ * (kPi / 180.f);
				float cy = std::cos(yaw);
				float sy = std::sin(yaw);
				float cp = std::cos(pit);
				float sp = std::sin(pit);

				//spherical offset (Z forward at yaw=0, pitch=0):(x,y,z) = (sy*cp, sp, cy*cp)
				glm::vec3 offset = { sy * cp, sp, cy * cp };
				return target_ + offset * dist_;
			}

			glm::mat4 view() const
			{
				return glm::lookAt(position(), target_, glm::vec3{0,1,0});
			}

			glm::mat4 proj() const
			{
				float aspect = (fbh_ > 0) ? (static_cast<float>(fbw_) / fbh_) : 1.0f;

				return glm::perspective(glm::radians(fovY_), aspect, zn_, zf_);
			}

			glm::mat4 vp() const { return proj() * view(); }

	private:
		static constexpr float clampPitch(float pDeg)
		{
			return std::clamp(pDeg, -89.0f, 89.0f);
		}

		glm::vec3 target_{ 0,0,0 };
		float dist_ = 5.0f;
		float yaw_ = 0.0f;
		float pitch_ = 0.0f;
		float fovY_ = 60.0f;
		float zn_ = 0.1f;
		float zf_ = 1000.0f;
		int fbw_ = 1;
		int fbh_ = 1;
	};		
}