#include "camera.hpp"

#include <algorithm>

namespace toaster
{
	Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
		: m_position(position), m_front(glm::vec3(0.0f, 0.0f, -1.0f)), m_worldUp(up), m_yaw(yaw), m_pitch(pitch)
	{
		updateCameraVectors();
	}

	glm::mat4 Camera::getViewMatrix() const
	{
		return glm::lookAt(m_position, m_position + m_front, m_up);
	}

	glm::mat4 Camera::getProjectionMatrix(float aspectRatio) const
	{
		glm::mat4 proj = glm::perspective(glm::radians(m_fov), aspectRatio, m_nearPlane, m_farPlane);
		// Flip Y for Vulkan coordinate system
		proj[1][1] *= -1.0f;
		return proj;
	}

	void Camera::processKeyboard(EMovement direction, float deltaTime)
	{
		const float velocity = m_movementSpeed * deltaTime;

		switch (direction)
		{
			case EMovement::eForward:
				m_position += m_front * velocity;
				break;
			case EMovement::eBackward:
				m_position -= m_front * velocity;
				break;
			case EMovement::eLeft:
				m_position -= m_right * velocity;
				break;
			case EMovement::eRight:
				m_position += m_right * velocity;
				break;
			case EMovement::eUp:
				m_position += m_worldUp * velocity;
				break;
			case EMovement::eDown:
				m_position -= m_worldUp * velocity;
				break;
		}
	}

	void Camera::processMouseMovement(float xOffset, float yOffset, bool constrainPitch)
	{
		xOffset *= m_mouseSensitivity;
		yOffset *= m_mouseSensitivity;

		m_yaw += xOffset;
		m_pitch += yOffset;

		if (constrainPitch)
		{
			m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);
		}

		updateCameraVectors();
	}

	void Camera::processMouseScroll(float yOffset)
	{
		m_fov -= yOffset;
		m_fov = std::clamp(m_fov, 1.0f, 120.0f);
	}

	void Camera::updateCameraVectors()
	{
		glm::vec3 front;
		front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
		front.y = sin(glm::radians(m_pitch));
		front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
		m_front = glm::normalize(front);

		m_right = glm::normalize(glm::cross(m_front, m_worldUp));
		m_up    = glm::normalize(glm::cross(m_right, m_front));
	}
}
