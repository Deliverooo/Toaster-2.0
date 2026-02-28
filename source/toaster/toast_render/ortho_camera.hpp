/*!
 * @file ortho_camera.hpp
 */
#pragma once

#include "camera.hpp"
#include "toaster/toast_lib/system_types.h"

#include <glm/gtc/quaternion.hpp>

namespace toaster
{
	class OrthoCamera : public Camera
	{
	public:
		OrthoCamera(float32 p_left, float32 p_right, float32 p_bottom, float32 p_top, float32 p_near, float32 p_far);

		[[nodiscard]] const glm::mat4 &getViewMatrix() const;
		[[nodiscard]] const glm::vec3 &getPosition() const;
		[[nodiscard]] float32          getRotation() const;

		void setPosition(const glm::vec3 &p_position);
		void setRotation(float32 p_rotation);

	private:
		static constexpr glm::vec3 c_worldUp{0.0f, 1.0f, 0.0f};

		void recalculateViewMatrix();

		glm::mat4 m_viewMatrix{1.0f};

		glm::vec3 m_position{0.0f};
		float32   m_rotation{0.0f};
	};
}
