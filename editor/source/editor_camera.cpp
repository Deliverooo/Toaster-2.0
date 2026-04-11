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
		glm::vec2       delta  = (mouse - m_initialMousePosition) * 0.003f;
		m_yaw                  += delta.x;
		m_pitch                += delta.y;

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

	// EditorCamera::EditorCamera(const float degFov, const float width, const float height, const float nearP, const float farP)
	// 	: Camera(glm::perspectiveFov(glm::radians(degFov), width, height, nearP, farP)),
	// 	  m_FocalPoint(0.0f), m_VerticalFOV(glm::radians(degFov)), m_NearClip(nearP), m_FarClip(farP)
	// {
	// 	Init();
	// }
	//
	// void EditorCamera::Init()
	// {
	// 	constexpr glm::vec3 position = {-5, 5, 5};
	// 	m_Distance                   = glm::distance(position, m_FocalPoint);
	//
	// 	m_Yaw   = 3.0f * glm::pi<float>() / 4.0f;
	// 	m_Pitch = glm::pi<float>() / 4.0f;
	//
	// 	m_Position                  = CalculatePosition();
	// 	const glm::quat orientation = GetOrientation();
	// 	m_Direction                 = glm::eulerAngles(orientation) * (180.0f / glm::pi<float>());
	// 	m_ViewMatrix                = glm::translate(glm::mat4(1.0f), m_Position) * glm::toMat4(orientation);
	// 	m_ViewMatrix                = glm::inverse(m_ViewMatrix);
	// }
	//
	// static void DisableMouse()
	// {
	// 	input::setCursorMode(input::ECursorMode::eDisabled);
	// 	// UI::SetInputEnabled(false);
	// }
	//
	// static void EnableMouse()
	// {
	// 	input::setCursorMode(input::ECursorMode::eNormal);
	// 	// UI::SetInputEnabled(true);
	// }
	//
	// void EditorCamera::OnUpdate(const float32 ts)
	// {
	// 	const glm::vec2 &mouse{input::getMouseX(), input::getMouseY()};
	// 	const glm::vec2  delta = (mouse - m_InitialMousePosition) * 0.002f;
	//
	// 	//HZ_CORE_WARN("EditorCamera=m_IsActive{}", m_IsActive);
	// 	if (!m_IsActive)
	// 	{
	// 		// if (!UI::IsInputEnabled())
	// 		// UI::SetInputEnabled(true);
	//
	// 		// return;
	// 	}
	//
	// 	if (input::isMouseButtonDown(input::EMouseButton::eRight) && !input::isKeyDown(input::EKeyCode::eLeftAlt))
	// 	{
	// 		m_CameraMode = CameraMode::FLYCAM;
	// 		DisableMouse();
	// 		const float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
	//
	// 		const float speed = GetCameraSpeed();
	//
	// 		if (input::isKeyDown(input::EKeyCode::eQ))
	// 			m_PositionDelta -= ts * speed * glm::vec3{0.f, yawSign, 0.f};
	// 		if (input::isKeyDown(input::EKeyCode::eE))
	// 			m_PositionDelta += ts * speed * glm::vec3{0.f, yawSign, 0.f};
	// 		if (input::isKeyDown(input::EKeyCode::eS))
	// 			m_PositionDelta -= ts * speed * m_Direction;
	// 		if (input::isKeyDown(input::EKeyCode::eW))
	// 			m_PositionDelta += ts * speed * m_Direction;
	// 		if (input::isKeyDown(input::EKeyCode::eA))
	// 			m_PositionDelta -= ts * speed * m_RightDirection;
	// 		if (input::isKeyDown(input::EKeyCode::eD))
	// 			m_PositionDelta += ts * speed * m_RightDirection;
	//
	// 		constexpr float maxRate{0.12f};
	// 		m_YawDelta   += glm::clamp(yawSign * delta.x * RotationSpeed(), -maxRate, maxRate);
	// 		m_PitchDelta += glm::clamp(delta.y * RotationSpeed(), -maxRate, maxRate);
	//
	// 		m_RightDirection = glm::cross(m_Direction, glm::vec3{0.f, yawSign, 0.f});
	//
	// 		m_Direction = glm::rotate(glm::normalize(glm::cross(glm::angleAxis(-m_PitchDelta, m_RightDirection),
	// 															glm::angleAxis(-m_YawDelta, glm::vec3{0.f, yawSign, 0.f}))), m_Direction);
	//
	// 		const float distance = glm::distance(m_FocalPoint, m_Position);
	// 		m_FocalPoint         = m_Position + GetForwardDirection() * distance;
	// 		m_Distance           = distance;
	// 	}
	// 	else if (input::isKeyDown(input::EKeyCode::eLeftAlt))
	// 	{
	// 		m_CameraMode = CameraMode::ARCBALL;
	//
	// 		if (input::isMouseButtonDown(input::EMouseButton::eMiddle))
	// 		{
	// 			DisableMouse();
	// 			MousePan(delta);
	// 		}
	// 		else if (input::isMouseButtonDown(input::EMouseButton::eLeft))
	// 		{
	// 			DisableMouse();
	// 			MouseRotate(delta);
	// 		}
	// 		else if (input::isMouseButtonDown(input::EMouseButton::eRight))
	// 		{
	// 			DisableMouse();
	// 			MouseZoom((delta.x + delta.y) * 0.1f);
	// 		}
	// 		else
	// 			EnableMouse();
	// 	}
	// 	else
	// 	{
	// 		EnableMouse();
	// 	}
	//
	// 	m_InitialMousePosition = mouse;
	// 	m_Position             += m_PositionDelta;
	// 	m_Yaw                  += m_YawDelta;
	// 	m_Pitch                += m_PitchDelta;
	//
	// 	if (m_CameraMode == CameraMode::ARCBALL)
	// 		m_Position = CalculatePosition();
	//
	// 	UpdateCameraView();
	// }
	//
	// float EditorCamera::GetCameraSpeed() const
	// {
	// 	float speed = m_NormalSpeed;
	// 	if (input::isKeyDown(input::EKeyCode::eLeftControl))
	// 		speed /= 2 - glm::log(m_NormalSpeed);
	// 	if (input::isKeyDown(input::EKeyCode::eLeftShift))
	// 		speed *= 2 - glm::log(m_NormalSpeed);
	//
	// 	return glm::clamp(speed, MIN_SPEED, MAX_SPEED);
	// }
	//
	// void EditorCamera::UpdateCameraView()
	// {
	// 	const float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
	//
	// 	// Extra step to handle the problem when the camera direction is the same as the up vector
	// 	const float cosAngle = glm::dot(GetForwardDirection(), GetUpDirection());
	// 	if (cosAngle * yawSign > 0.99f)
	// 		m_PitchDelta = 0.f;
	//
	// 	const glm::vec3 lookAt = m_Position + GetForwardDirection();
	// 	m_Direction            = glm::normalize(lookAt - m_Position);
	// 	m_Distance             = glm::distance(m_Position, m_FocalPoint);
	// 	m_ViewMatrix           = glm::lookAt(m_Position, lookAt, glm::vec3{0.f, yawSign, 0.f});
	//
	// 	//damping for smooth camera
	// 	m_YawDelta      *= 0.6f;
	// 	m_PitchDelta    *= 0.6f;
	// 	m_PositionDelta *= 0.8f;
	// }
	//
	// void EditorCamera::Focus(const glm::vec3 &focusPoint)
	// {
	// 	m_FocalPoint = focusPoint;
	// 	m_CameraMode = CameraMode::FLYCAM;
	// 	if (m_Distance > m_MinFocusDistance)
	// 	{
	// 		m_Distance -= m_Distance - m_MinFocusDistance;
	// 		m_Position = m_FocalPoint - GetForwardDirection() * m_Distance;
	// 	}
	// 	m_Position = m_FocalPoint - GetForwardDirection() * m_Distance;
	// 	UpdateCameraView();
	// }
	//
	// std::pair<float, float> EditorCamera::PanSpeed() const
	// {
	// 	const float x       = glm::min(float(m_ViewportRight - m_ViewportLeft) / 1000.0f, 2.4f); // max = 2.4f
	// 	const float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;
	//
	// 	const float y       = glm::min(float(m_ViewportBottom - m_ViewportTop) / 1000.0f, 2.4f); // max = 2.4f
	// 	const float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;
	//
	// 	return {xFactor, yFactor};
	// }
	//
	// float EditorCamera::RotationSpeed() const
	// {
	// 	return 0.3f;
	// }
	//
	// float EditorCamera::ZoomSpeed() const
	// {
	// 	float distance = m_Distance * 0.2f;
	// 	distance       = glm::max(distance, 0.0f);
	// 	float speed    = distance * distance;
	// 	speed          = glm::min(speed, 50.0f); // max speed = 50
	// 	return speed;
	// }
	//
	// void EditorCamera::OnEvent(Event &event)
	// {
	// 	EventDispatcher dispatcher(event);
	// 	dispatcher.dispatch<MouseScrollEvent>(TST_BIND_EVENT_FN(EditorCamera::OnMouseScroll));
	// }
	//
	// bool EditorCamera::OnMouseScroll(MouseScrollEvent &e)
	// {
	// 	if (input::isMouseButtonDown(input::EMouseButton::eRight))
	// 	{
	// 		m_NormalSpeed += e.getScrollY() * 0.3f * m_NormalSpeed;
	// 		m_NormalSpeed = glm::clamp(m_NormalSpeed, MIN_SPEED, MAX_SPEED);
	// 	}
	// 	else
	// 	{
	// 		MouseZoom(e.getScrollY() * 0.1f);
	// 		UpdateCameraView();
	// 	}
	//
	// 	return true;
	// }
	//
	// void EditorCamera::MousePan(const glm::vec2 &delta)
	// {
	// 	auto [xSpeed, ySpeed] = PanSpeed();
	// 	m_FocalPoint          -= GetRightDirection() * delta.x * xSpeed * m_Distance;
	// 	m_FocalPoint          += GetUpDirection() * delta.y * ySpeed * m_Distance;
	// }
	//
	// void EditorCamera::MouseRotate(const glm::vec2 &delta)
	// {
	// 	const float yawSign = GetUpDirection().y < 0.0f ? -1.0f : 1.0f;
	// 	m_YawDelta          += yawSign * delta.x * RotationSpeed();
	// 	m_PitchDelta        += delta.y * RotationSpeed();
	// }
	//
	// void EditorCamera::MouseZoom(float delta)
	// {
	// 	m_Distance                 -= delta * ZoomSpeed();
	// 	const glm::vec3 forwardDir = GetForwardDirection();
	// 	m_Position                 = m_FocalPoint - forwardDir * m_Distance;
	// 	if (m_Distance < 1.0f)
	// 	{
	// 		m_FocalPoint += forwardDir * m_Distance;
	// 		m_Distance   = 1.0f;
	// 	}
	// 	m_PositionDelta += delta * ZoomSpeed() * forwardDir;
	// }
	//
	// glm::vec3 EditorCamera::GetUpDirection() const
	// {
	// 	return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
	// }
	//
	// glm::vec3 EditorCamera::GetRightDirection() const
	// {
	// 	return glm::rotate(GetOrientation(), glm::vec3(1.f, 0.f, 0.f));
	// }
	//
	// glm::vec3 EditorCamera::GetForwardDirection() const
	// {
	// 	return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
	// }
	//
	// glm::vec3 EditorCamera::CalculatePosition() const
	// {
	// 	return m_FocalPoint - GetForwardDirection() * m_Distance + m_PositionDelta;
	// }
	//
	// glm::quat EditorCamera::GetOrientation() const
	// {
	// 	return {{-m_Pitch - m_PitchDelta, -m_Yaw - m_YawDelta, 0.0f}};
	// }
}
