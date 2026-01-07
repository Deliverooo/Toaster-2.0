#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace toaster
{
	class Camera
	{
	public:
		enum class EMovement
		{
			eForward,
			eBackward,
			eLeft,
			eRight,
			eUp,
			eDown
		};

		Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = -90.0f, float pitch = 0.0f);

		[[nodiscard]] glm::mat4 getViewMatrix() const;
		[[nodiscard]] glm::mat4 getProjectionMatrix(float aspectRatio) const;

		void processKeyboard(EMovement direction, float deltaTime);
		void processMouseMovement(float xOffset, float yOffset, bool constrainPitch = true);
		void processMouseScroll(float yOffset);

		[[nodiscard]] glm::vec3 getPosition() const { return m_position; }
		[[nodiscard]] glm::vec3 getFront() const { return m_front; }
		[[nodiscard]] float     getFov() const { return m_fov; }
		[[nodiscard]] float     getNearPlane() const { return m_nearPlane; }
		[[nodiscard]] float     getFarPlane() const { return m_farPlane; }

		void setPosition(const glm::vec3 &position) { m_position = position; }
		void setMovementSpeed(float speed) { m_movementSpeed = speed; }
		void setMouseSensitivity(float sensitivity) { m_mouseSensitivity = sensitivity; }
		void setFov(float fov) { m_fov = fov; }
		void setNearPlane(float nearPlane) { m_nearPlane = nearPlane; }
		void setFarPlane(float farPlane) { m_farPlane = farPlane; }

	private:
		void updateCameraVectors();

		glm::vec3 m_position;
		glm::vec3 m_front;
		glm::vec3 m_up;
		glm::vec3 m_right;
		glm::vec3 m_worldUp;

		float m_yaw;
		float m_pitch;

		float m_movementSpeed{5.0f};
		float m_mouseSensitivity{0.1f};
		float m_fov{45.0f};
		float m_nearPlane{0.1f};
		float m_farPlane{1000.0f};
	};
}
