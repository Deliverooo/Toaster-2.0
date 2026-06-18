#pragma once

#include <toast_kernel/application.hpp>
#include <toast_kernel/fp_camera.hpp>

#include <toast_render/dynamic_renderer_2d.hpp>
#include <toast_render/graphics_state.hpp>
#include <toast_render/image.hpp>

#include <toast_lib/events/key_event.hpp>
#include <toast_lib/events/mouse_event.hpp>
#include <toast_lib/events/window_event.hpp>

#include <toast_lib/camera.hpp>

#include "image_viewer_camera.hpp"

namespace tst = toaster;

namespace img
{
	class NewLayer : public tst::IAppLayer
	{
	public:
		auto onInit() -> void override;
		auto onDestroy() -> void override;
		auto onUpdate(float32 p_dt) -> void override;
		auto onEvent(tst::Event &p_event) -> void override;
		auto onResize(tsm::uint2 p_size) -> void override;

	private:
		auto _onKeyPressEvent(tst::KeyPressEvent &p_key_press_event) -> bool;
		auto _onMouseScrollEvent(tst::MouseScrollEvent &p_mouse_scroll_event) -> bool;
		auto _onFileDropEvent(tst::WindowFileDropEvent &p_file_drop_event) -> bool;

		// It is good practice to store the viewport's current size
		tsm::uint2 m_viewportSize{0u};

		tst::render::GraphicsStateUnique m_compositeGraphicsState{nullptr};

		tst::render::ImageHandle m_image{nullptr};

		tst::gpu::RawImageHandle                       m_MSAAColourImage{nullptr};
		tst::UniquePtr<tst::render::DynamicRenderer2D> m_renderer2D{nullptr};

		ImageViewerCamera m_camera{};
	};
}
