#pragma once

#include "toast_lib/events/key_event.hpp"
#include "toast_lib/events/mouse_event.hpp"
#include "toast_lib/events/window_event.hpp"
#include "toast_render/ortho_camera.hpp"

namespace toaster
{
	class OrthoCameraController
	{
	public:
		OrthoCameraController(float32 p_aspect_ratio, bool p_rotate_camera = false);

		void onUpdate(float32 p_dt);
		void onEvent(Event &p_event);

		void onResize(float32 p_width, float32 p_height);

		[[nodiscard]] const OrthoCamera &getCamera() const;

	private:
		bool onMouseScrollEvent(MouseScrollEvent &e);
		bool onWindowResizeEvent(WindowResizeEvent &e);

		float32     m_aspectRatio;
		float32     m_zoom{1.0f};
		OrthoCamera m_camera;

		bool m_rotateCamera;

		glm::vec3 m_cameraPosition{0.0f, 0.0f, 0.0f};
		float32   m_cameraRotation{0.0f};

		float32 m_movementSpeed{5.0f};
		float32 m_rotationSpeed{180.0f};
	};
}
