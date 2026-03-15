#include "ortho_camera_controller.hpp"
#include "input.hpp"

namespace toaster
{
	OrthoCameraController::OrthoCameraController(float32 p_aspect_ratio, bool p_rotate_camera) : m_aspectRatio(p_aspect_ratio),
																								 m_camera(-p_aspect_ratio * m_zoom, p_aspect_ratio * m_zoom, -m_zoom,
																										  m_zoom, 0.1f, 10.0f), m_rotateCamera(p_rotate_camera)
	{
	}

	void OrthoCameraController::onUpdate(float32 p_dt)
	{
		if (input::isKeyDown(input::EKeyCode::eW))
		{
			m_cameraPosition.x += -glm::sin(glm::radians(m_cameraRotation)) * m_movementSpeed * p_dt;
			m_cameraPosition.y += glm::cos(glm::radians(m_cameraRotation)) * m_movementSpeed * p_dt;
		}
		if (input::isKeyDown(input::EKeyCode::eS))
		{
			m_cameraPosition.x -= -glm::sin(glm::radians(m_cameraRotation)) * m_movementSpeed * p_dt;
			m_cameraPosition.y -= glm::cos(glm::radians(m_cameraRotation)) * m_movementSpeed * p_dt;
		}
		if (input::isKeyDown(input::EKeyCode::eA))
		{
			m_cameraPosition.x -= glm::cos(glm::radians(m_cameraRotation)) * m_movementSpeed * p_dt;
			m_cameraPosition.y -= glm::sin(glm::radians(m_cameraRotation)) * m_movementSpeed * p_dt;
		}
		if (input::isKeyDown(input::EKeyCode::eD))
		{
			m_cameraPosition.x += glm::cos(glm::radians(m_cameraRotation)) * m_movementSpeed * p_dt;
			m_cameraPosition.y += glm::sin(glm::radians(m_cameraRotation)) * m_movementSpeed * p_dt;
		}

		if (m_rotateCamera)
		{
			if (input::isKeyDown(input::EKeyCode::eQ))
				m_cameraRotation += m_rotationSpeed * p_dt;
			if (input::isKeyDown(input::EKeyCode::eE))
				m_cameraRotation -= m_rotationSpeed * p_dt;

			m_camera.setRotation(m_cameraRotation);
		}

		m_camera.setPosition(m_cameraPosition);
	}

	void OrthoCameraController::onEvent(Event &p_event)
	{
		EventDispatcher eventDispatcher(p_event);
		eventDispatcher.dispatch<MouseScrollEvent>(TST_BIND_EVENT_FN(OrthoCameraController::onMouseScrollEvent));
		eventDispatcher.dispatch<WindowResizeEvent>(TST_BIND_EVENT_FN(OrthoCameraController::onWindowResizeEvent));
	}

	void OrthoCameraController::onResize(float32 p_width, float32 p_height)
	{
		m_aspectRatio = p_width / p_height;
		m_camera.setProjectionMatrix(-m_aspectRatio * m_zoom, m_aspectRatio * m_zoom, -m_zoom, m_zoom, 0.1f, 10.0f);
	}

	const OrthoCamera &OrthoCameraController::getCamera() const
	{
		return m_camera;
	}

	bool OrthoCameraController::onMouseScrollEvent(MouseScrollEvent &e)
	{
		m_zoom -= e.getScrollY() * 0.25f;
		m_zoom = std::max(m_zoom, 0.25f);
		m_camera.setProjectionMatrix(-m_aspectRatio * m_zoom, m_aspectRatio * m_zoom, -m_zoom, m_zoom, 0.1f, 10.0f);

		return false;
	}

	bool OrthoCameraController::onWindowResizeEvent(WindowResizeEvent &e)
	{
		onResize(static_cast<float32>(e.getWidth()), static_cast<float32>(e.getHeight()));
		return false;
	}
}
