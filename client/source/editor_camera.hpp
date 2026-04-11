#pragma once

#include "toast_lib/camera.hpp"
#include "toast_lib/events/mouse_event.hpp"

#include <glm/gtx/quaternion.hpp>

namespace toaster
{
	class EditorCamera final : public Camera
	{
	public:
		EditorCamera() = default;
		EditorCamera(float32 p_fov, float32 p_aspectRatio, float32 p_near, float32 p_far);

		void onUpdate(float32 p_dt);
		void onEvent(Event &p_event);

		void setViewportSize(float32 p_width, float32 p_height);

		[[nodiscard]] const glm::mat4 &getViewMatrix() const;
		[[nodiscard]] glm::mat4        getViewProjection() const;

		[[nodiscard]] const glm::vec3 &getUpDirection() const;
		[[nodiscard]] const glm::vec3 &getRightDirection() const;
		[[nodiscard]] const glm::vec3 &getForwardDirection() const;

		[[nodiscard]] const glm::vec3 &getPosition() const;
		[[nodiscard]] glm::quat        getOrientation() const;

		[[nodiscard]] float32 getPitch() const;
		[[nodiscard]] float32 getYaw() const;

	private:
		void _updateProjection();
		void _updateView();

		bool _onMouseScrollEvent(MouseScrollEvent &p_event);
		void _mouseZoom(float32 p_delta);

		glm::mat4 m_viewMatrix{1.0f};
		glm::vec3 m_position{0.0f, 0.0f, 0.0f};

		glm::vec3 m_forward{0.0f, 0.0f, 1.0f};
		glm::vec3 m_right{1.0f, 0.0f, 0.0f};
		glm::vec3 m_up{0.0f, -1.0f, 0.0f};

		glm::vec2 m_initialMousePosition{0.0f};

		float32 m_yaw{90.0f};
		float32 m_pitch{0.0f};

		float32 m_fov{45.0f};
		float32 m_aspectRatio{1.0f};
		float32 m_zNear{0.1f};
		float32 m_zFar{1000.0f};

		float32 m_zoom{1.0f};
	};
}
