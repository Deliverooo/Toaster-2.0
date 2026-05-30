#pragma once

#include "toast_lib/camera.hpp"
#include "toast_lib/events/mouse_event.hpp"

#include "toast_lib/ptr.hpp"

namespace toaster
{
	class InputContext;

	class FPCamera : public Camera
	{
	public:
		FPCamera() = default;
		FPCamera(InputContext *p_ctx, float32 p_fov, float32 p_aspectRatio, float32 p_near, float32 p_far);

		auto onUpdate(float32 p_dt) -> void;
		auto onEvent(Event &p_event) -> void;

		auto setViewportSize(float32 p_width, float32 p_height) -> void;

		[[nodiscard]] auto getViewMatrix() const -> tsm::float4x4;
		[[nodiscard]] auto getRotationMatrix() const -> tsm::float4x4; // Thank you very much -> https://vkguide.dev/docs/new_chapter_5/interactive_camera/
		[[nodiscard]] auto getViewProjection() const -> tsm::float4x4;

		[[nodiscard]] auto getForwardDirection() const -> tsm::float3;
		[[nodiscard]] auto getRightDirection() const -> tsm::float3;
		[[nodiscard]] auto getUpDirection() const -> tsm::float3;

		[[nodiscard]] auto getPosition() const -> const tsm::float3 &;

		[[nodiscard]] auto getPitch() const -> float32;
		[[nodiscard]] auto getYaw() const -> float32;

	private:
		auto _updateProjection() -> void;
		auto _onMouseScrollEvent(MouseScrollEvent &p_event) -> bool;

		NonOwningPtr<InputContext> m_ctx{nullptr};

		tsm::float3 m_position{0.0f, 1.0f, 3.0f};

		static constexpr tsm::float3 c_forwardDir{0.0f, 0.0f, -1.0f};
		static constexpr tsm::float3 c_rightDir{1.0f, 0.0f, 0.0f};
		static constexpr tsm::float3 c_upDir{0.0f, 1.0f, 0.0f};

		tsm::float2 m_initialMousePosition{0.0f};

		float32 m_yaw{0.0f};
		float32 m_pitch{0.0f};

		float32 m_fov{45.0f};
		float32 m_aspectRatio{1.0f};
		float32 m_zNear{0.1f};
		float32 m_zFar{1000.0f};

		float32 m_zoom{1.0f};
	};
}
