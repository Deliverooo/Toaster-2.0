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

		// float32 m_viewportWidth{1920u};
		// float32 m_viewportHeight{1080u};
	};

	// enum class CameraMode
	// {
	// 	NONE, FLYCAM, ARCBALL
	// };
	//
	// class EditorCamera : public Camera
	// {
	// public:
	// 	EditorCamera(float degFov, float width, float height, float nearP, float farP);
	// 	void Init();
	//
	// 	void Focus(const glm::vec3 &focusPoint);
	// 	void OnUpdate(float32 ts);
	// 	void OnEvent(Event &e);
	//
	// 	bool IsActive() const { return m_IsActive; }
	// 	void SetActive(bool active) { m_IsActive = active; }
	//
	// 	CameraMode GetCurrentMode() const { return m_CameraMode; }
	//
	// 	float GetDistance() const { return m_Distance; }
	// 	void  SetDistance(float distance) { m_Distance = distance; }
	//
	// 	const glm::vec3 &GetFocalPoint() const { return m_FocalPoint; }
	//
	// 	void SetViewportBounds(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom)
	// 	{
	// 		if (m_ViewportLeft == left && m_ViewportTop == top && m_ViewportRight == right && m_ViewportBottom == bottom)
	// 			return;
	//
	// 		if ((right - left) != (m_ViewportRight - m_ViewportLeft) || (bottom - top) != (m_ViewportBottom - m_ViewportTop))
	// 		{
	// 			float width  = (float) (right - left);
	// 			float height = (float) (bottom - top);
	// 			if (width > 0.0f && height > 0.0f)
	// 			{
	// 				m_projection = glm::perspectiveFov(m_VerticalFOV, width, height, m_NearClip, m_FarClip);
	// 			}
	// 		}
	//
	// 		m_ViewportLeft   = left;
	// 		m_ViewportTop    = top;
	// 		m_ViewportRight  = right;
	// 		m_ViewportBottom = bottom;
	// 	}
	//
	// 	const glm::mat4 &GetViewMatrix() const { return m_ViewMatrix; }
	// 	glm::mat4        GetViewProjection() const { return getProjectionMatrix() * m_ViewMatrix; }
	//
	// 	glm::vec3 GetUpDirection() const;
	// 	glm::vec3 GetRightDirection() const;
	// 	glm::vec3 GetForwardDirection() const;
	//
	// 	const glm::vec3 &GetPosition() const { return m_Position; }
	//
	// 	glm::quat GetOrientation() const;
	//
	// 	[[nodiscard]] float GetVerticalFOV() const { return m_VerticalFOV; }
	// 	[[nodiscard]] float GetAspectRatio() const { return m_AspectRatio; }
	// 	[[nodiscard]] float GetNearClip() const { return m_NearClip; }
	// 	[[nodiscard]] float GetFarClip() const { return m_FarClip; }
	// 	[[nodiscard]] float GetPitch() const { return m_Pitch; }
	// 	[[nodiscard]] float GetYaw() const { return m_Yaw; }
	// 	[[nodiscard]] float GetCameraSpeed() const;
	//
	// private:
	// 	void UpdateCameraView();
	//
	// 	bool OnMouseScroll(MouseScrollEvent &e);
	//
	// 	void MousePan(const glm::vec2 &delta);
	// 	void MouseRotate(const glm::vec2 &delta);
	// 	void MouseZoom(float delta);
	//
	// 	glm::vec3 CalculatePosition() const;
	//
	// 	std::pair<float, float> PanSpeed() const;
	// 	float                   RotationSpeed() const;
	// 	float                   ZoomSpeed() const;
	//
	// private:
	// 	glm::mat4 m_ViewMatrix;
	// 	glm::vec3 m_Position, m_Direction, m_FocalPoint;
	//
	// 	// Perspective projection params
	// 	float m_VerticalFOV, m_AspectRatio, m_NearClip, m_FarClip;
	//
	// 	bool      m_IsActive = false;
	// 	bool      m_Panning, m_Rotating;
	// 	glm::vec2 m_InitialMousePosition{};
	// 	glm::vec3 m_InitialFocalPoint, m_InitialRotation;
	//
	// 	float m_Distance;
	// 	float m_NormalSpeed{0.002f};
	//
	// 	float     m_Pitch,        m_Yaw;
	// 	float     m_PitchDelta{}, m_YawDelta{};
	// 	glm::vec3 m_PositionDelta{};
	// 	glm::vec3 m_RightDirection{};
	//
	// 	CameraMode m_CameraMode{CameraMode::ARCBALL};
	//
	// 	float m_MinFocusDistance{100.0f};
	//
	// 	uint32_t m_ViewportLeft   = 0;
	// 	uint32_t m_ViewportTop    = 0;
	// 	uint32_t m_ViewportRight  = 1280;
	// 	uint32_t m_ViewportBottom = 720;
	//
	// 	constexpr static float MIN_SPEED{0.0005f}, MAX_SPEED{2.0f};
	// 	friend class EditorLayer;
	// };
}
