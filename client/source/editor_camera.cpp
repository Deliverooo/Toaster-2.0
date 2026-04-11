#include "editor_camera.hpp"

#include "toast_kernel/input.hpp"

namespace toaster
{
	EditorCamera::EditorCamera(float32 p_fov, float32 p_aspectRatio, float32 p_near, float32 p_far) : Camera(glm::perspective(p_fov, p_aspectRatio, p_near, p_far)),
																									  m_fov(p_fov), m_aspectRatio(p_aspectRatio), m_zNear(p_near),
																									  m_zFar(p_far)
	{
		_updateView();
	}

	void EditorCamera::onUpdate(float32 p_dt)
	{
		glm::vec3 delta_position{0.0f};
		if (input::isKeyDown(input::EKeyCode::eW))
			delta_position += glm::normalize(glm::vec3(m_forward.x, 0.0f, m_forward.z));
		if (input::isKeyDown(input::EKeyCode::eA))
			delta_position -= glm::normalize(m_right);
		if (input::isKeyDown(input::EKeyCode::eS))
			delta_position -= glm::normalize(glm::vec3(m_forward.x, 0.0f, m_forward.z));
		if (input::isKeyDown(input::EKeyCode::eD))
			delta_position += glm::normalize(m_right);

		delta_position = glm::normalize(delta_position) * p_dt;
		m_position     += delta_position;

		const glm::vec2 mouse{input::getMouseX(), input::getMouseY()};
		glm::vec2       delta = (mouse - m_initialMousePosition) * 0.003f;
		m_yaw                 += delta.x;
		m_pitch               += delta.y;

		m_initialMousePosition = mouse;

		_updateView();

		// if (input::isKeyDown(input::EKeyCode::eLeftAlt))
		// {

		// }
		// _updateView();
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

	const glm::mat4 &EditorCamera::getViewMatrix() const
	{
		return m_viewMatrix;
	}

	glm::mat4 EditorCamera::getViewProjection() const
	{
		return m_projection * m_viewMatrix;
	}

	const glm::vec3 &EditorCamera::getUpDirection() const
	{
		return m_up;
	}

	const glm::vec3 &EditorCamera::getRightDirection() const
	{
		return m_right;
	}

	const glm::vec3 &EditorCamera::getForwardDirection() const
	{
		return m_forward;
	}

	const glm::vec3 &EditorCamera::getPosition() const
	{
		return m_position;
	}

	glm::quat EditorCamera::getOrientation() const
	{
		return {{-m_pitch, -m_yaw, 0.0f}};
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
		m_projection = glm::perspective(glm::radians(m_fov), m_aspectRatio, m_zNear, m_zFar);
	}

	void EditorCamera::_updateView()
	{
		glm::vec3 dir{0.0f};
		dir.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
		dir.y = sin(glm::radians(m_pitch));
		dir.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));

		m_forward = glm::normalize(dir);
		m_right   = glm::normalize(glm::cross(dir, glm::vec3{0.0f, -1.0f, 0.0f}));
		m_up      = glm::normalize(glm::cross(m_right, m_forward));

		m_viewMatrix       = glm::lookAt(m_position, m_position + m_forward, m_up);
		m_viewMatrix[1][1] *= -1.0f;
	}

	bool EditorCamera::_onMouseScrollEvent(MouseScrollEvent &p_event)
	{
		float32 delta = p_event.getScrollY() * 0.1f;
		_mouseZoom(delta);
		_updateView();

		return false;
	}

	void EditorCamera::_mouseZoom(float32 p_delta)
	{
		m_fov += p_delta;
	}
}
