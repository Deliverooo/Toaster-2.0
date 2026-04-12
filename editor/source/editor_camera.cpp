#include "editor_camera.hpp"

#include "toast_kernel/input.hpp"
#include "toast_lib/logging.hpp"

namespace toaster
{
	EditorCamera::EditorCamera(float32 p_fov, float32 p_aspectRatio, float32 p_near, float32 p_far) : Camera(glm::perspective(glm::radians(p_fov), p_aspectRatio, p_near,
																															  p_far)), m_fov(p_fov),
																									  m_aspectRatio(p_aspectRatio), m_zNear(p_near), m_zFar(p_far)
	{
	}

	void EditorCamera::onUpdate(float32 p_dt)
	{
		if (input::isMouseButtonDown(input::EMouseButton::eRight))
		{
			if (input::getCursorMode() != input::ECursorMode::eDisabled)
				input::setCursorMode(input::ECursorMode::eDisabled);

			glm::vec3 delta_position{0.0f};
			if (input::isKeyDown(input::EKeyCode::eW))
				delta_position -= c_forwardDir;
			if (input::isKeyDown(input::EKeyCode::eA))
				delta_position -= c_rightDir;
			if (input::isKeyDown(input::EKeyCode::eS))
				delta_position += c_forwardDir;
			if (input::isKeyDown(input::EKeyCode::eD))
				delta_position += c_rightDir;

			delta_position = ((glm::length(delta_position) == 0.0f) ? glm::vec3{0.0f} : glm::normalize(delta_position)) * p_dt;
			m_position     += glm::vec3{getRotationMatrix() * glm::vec4{delta_position, 0.0f}} * 10.0f;
			if (input::isKeyDown(input::EKeyCode::eSpace))
				m_position += c_upDir * p_dt * 10.0f;
			if (input::isKeyDown(input::EKeyCode::eLeftShift))
				m_position -= c_upDir * p_dt * 10.0f;

			const glm::vec2 mouse{input::getMouseX(), input::getMouseY()};
			const glm::vec2 delta{(mouse - m_initialMousePosition) * 0.002f};
			m_yaw   += delta.x;
			m_pitch += delta.y;
			if (m_pitch > glm::radians(89.0f))
				m_pitch = glm::radians(89.0f);
			if (m_pitch < glm::radians(-89.0f))
				m_pitch = glm::radians(-89.0f);

			m_initialMousePosition = mouse;
		}
		else
		{
			if (input::getCursorMode() != input::ECursorMode::eNormal)
				input::setCursorMode(input::ECursorMode::eNormal);

			const glm::vec2 mouse{input::getMouseX(), input::getMouseY()};
			m_initialMousePosition = mouse;
		}
	}

	void EditorCamera::onEvent(Event &p_event)
	{
		EventDispatcher dispatcher{p_event};
		dispatcher.dispatch<MouseScrollEvent>(TST_BIND_EVENT_FN(EditorCamera::_onMouseScrollEvent));
	}

	void EditorCamera::setViewportSize(float32 p_width, float32 p_height)
	{
		m_aspectRatio = p_width / p_height;
		_updateProjection();
	}

	glm::mat4 EditorCamera::getViewMatrix() const
	{
		const glm::mat4 cameraTranslation{glm::translate(glm::mat4{1.0f}, m_position)};
		const glm::mat4 cameraRotation{getRotationMatrix()};
		return glm::inverse(cameraTranslation * cameraRotation);
	}

	glm::mat4 EditorCamera::getRotationMatrix() const
	{
		glm::quat pitchRotation{glm::angleAxis(m_pitch, glm::vec3{1.f, 0.f, 0.f})};
		glm::quat yawRotation{glm::angleAxis(m_yaw, glm::vec3{0.f, -1.f, 0.f})};

		return glm::toMat4(yawRotation) * glm::toMat4(pitchRotation);
	}

	glm::mat4 EditorCamera::getViewProjection() const
	{
		return m_projection * getViewMatrix();
	}

	glm::vec3 EditorCamera::getForwardDirection() const
	{
		return getRotationMatrix() * glm::vec4{c_forwardDir, 0.0f};
	}

	glm::vec3 EditorCamera::getRightDirection() const
	{
		return getRotationMatrix() * glm::vec4{c_rightDir, 0.0f};
	}

	glm::vec3 EditorCamera::getUpDirection() const
	{
		return getRotationMatrix() * glm::vec4{c_upDir, 0.0f};
	}

	const glm::vec3 &EditorCamera::getPosition() const
	{
		return m_position;
	}

	float32 EditorCamera::getPitch() const
	{
		return m_pitch;
	}

	float32 EditorCamera::getYaw() const
	{
		return m_yaw;
	}

	void EditorCamera::_updateProjection()
	{
		m_projection = glm::perspective(glm::radians(m_fov) * m_zoom, m_aspectRatio, m_zNear, m_zFar);
	}

	bool EditorCamera::_onMouseScrollEvent(MouseScrollEvent &p_event)
	{
		m_zoom -= p_event.getScrollY() * 0.04f;
		_updateProjection();

		return false;
	}
}
