#pragma once

#include <toast_kernel/input.hpp>
#include <toast_lib/camera.hpp>
#include <toast_lib/events/event.hpp>
#include <toast_lib/events/mouse_event.hpp>

namespace tst = toaster;

namespace img
{
	class ImageViewerCamera : public tst::Camera
	{
	public:
		ImageViewerCamera() = default;
		ImageViewerCamera(tst::InputContext *p_ctx, float32 p_fov, float32 p_aspectRatio, float32 p_near, float32 p_far);

		auto onUpdate(float32 p_dt) -> void;
		auto onEvent(tst::Event &p_event) -> void;

		auto onResize(tsm::uint2 p_size) -> void;

		[[nodiscard]] auto XM_CALLCONV getViewMatrix() const -> Dx::XMMATRIX;
		[[nodiscard]] auto XM_CALLCONV getRotationMatrix() const -> Dx::XMMATRIX; // Thank you very much -> https://vkguide.dev/docs/new_chapter_5/interactive_camera/
		[[nodiscard]] auto XM_CALLCONV getViewProjection() const -> Dx::XMMATRIX;

		[[nodiscard]] auto XM_CALLCONV getForwardDirection() const -> Dx::XMVECTOR;
		[[nodiscard]] auto XM_CALLCONV getRightDirection() const -> Dx::XMVECTOR;
		[[nodiscard]] auto XM_CALLCONV getUpDirection() const -> Dx::XMVECTOR;

		[[nodiscard]] auto XM_CALLCONV getPosition() const -> Dx::XMVECTOR;

		[[nodiscard]] auto getPitch() const -> float32;
		[[nodiscard]] auto getYaw() const -> float32;

	private:
		auto _updateProjection() -> void;
		auto _onMouseScrollEvent(toaster::MouseScrollEvent &p_event) -> bool;

		tst::NonOwningPtr<tst::InputContext> m_ctx{nullptr};

		Dx::XMFLOAT3 m_position{0.0f, 0.0f, 3.0f};

		static constexpr Dx::XMVECTORF32 c_forwardDir{.f{0.0f, 0.0f, -1.0f, 0.0f}};
		static constexpr Dx::XMVECTORF32 c_rightDir{.f{1.0f, 0.0f, 0.0f, 0.0f}};
		static constexpr Dx::XMVECTORF32 c_upDir{.f{0.0f, 1.0f, 0.0f, 0.0f}};

		tsm::float2 m_initialMousePosition{0.0f};

		float32 m_yaw{tsm::radians(0.0f)};
		float32 m_pitch{tsm::radians(0.0f)};

		float32 m_fov{45.0f};
		float32 m_aspectRatio{1.0f};
		float32 m_zNear{0.1f};
		float32 m_zFar{1000.0f};

		float32 m_zoom{1.0f};
	};
}
