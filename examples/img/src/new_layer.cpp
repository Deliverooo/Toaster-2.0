#include "img/new_layer.hpp"

#include <toast_kernel/fp_camera.hpp>

#include <toast_math/colours.hpp>

#include <toast_lib/os/terminal.hpp>

#include <toast_gpu/vk/vk_swapchain.hpp>
#include <toast_render/render_context.hpp>

namespace img
{
	auto NewLayer::onInit() -> void
	{
		// Get the initial viewport size
		m_viewportSize = m_app->getWindow().getRenderAreaSize();

		// The actual image we are viewing
		m_image = m_renderCtx->createImageRef(m_app->getCommandLineArgs()->get<tst::String>("--image"));

		m_MSAAColourImage = m_renderCtx->createMultisampleAttachmentImage(m_viewportSize, vk::ImageAspectFlagBits::eColor);

		tst::render::DynamicRenderer2DSpecInfo renderer_2d_spec_info{};
		renderer_2d_spec_info.maxQuads            = 10u;
		renderer_2d_spec_info.msaa                = true;
		renderer_2d_spec_info.overrideAttachments = true;
		m_renderer2D                              = m_renderCtx->createUnique<tst::render::DynamicRenderer2D>(renderer_2d_spec_info);

		m_camera = ImageViewerCamera{m_inputCtx, 60.0f, m_viewportSize.aspect(), 0.1f, 1000.0f};
	}

	auto NewLayer::onDestroy() -> void
	{
		// Trigger layer-specific destruction here
	}

	auto NewLayer::onUpdate(float32 p_dt) -> void
	{
		m_camera.onUpdate(p_dt);

		// Get the swapchain's rendering info from the window and set the clear colour to white, while using no depth because the scene renderer provides that.
		auto rendering_info{m_app->getWindow().getSwapchainRenderingInfo(true, {0.0f, 0.0f, 0.05f, 1.0f}, true)};

		// Bind the render context's descriptor heap. TODO: Probably don't expose ts to the client
		m_renderCtx->getDescriptorHeap()->bind();

		rendering_info.colourAttachments[0].image = m_MSAAColourImage;

		m_renderer2D->submitQuad(Dx::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), Dx::XMVectorSet(-m_image->getSpecInfo().size.aspect() * 2.0f, 2.0f, 2.0f, 1.0f), m_image);
		m_renderer2D->render(m_camera.getViewMatrix(), m_camera.getProjectionMatrix(), &rendering_info);
		// m_renderer2D->render(Dx::XMLoadFloat4x4(&m_viewMatrix), m_camera.getProjectionMatrix(), &rendering_info);
	}

	auto NewLayer::onEvent(tst::Event &p_event) -> void
	{
		// Handle events here
		tst::EventDispatcher event_dispatcher{p_event};
		event_dispatcher.dispatch<tst::KeyPressEvent>(TST_BIND_EVENT_FN(NewLayer::_onKeyPressEvent));
		// event_dispatcher.dispatch<tst::MouseScrollEvent>(TST_BIND_EVENT_FN(NewLayer::_onMouseScrollEvent));
		event_dispatcher.dispatch<tst::WindowFileDropEvent>(TST_BIND_EVENT_FN(NewLayer::_onFileDropEvent));

		m_camera.onEvent(p_event);
	}

	auto NewLayer::onResize(tsm::uint2 p_size) -> void
	{
		m_viewportSize = p_size;
		m_MSAAColourImage->resize(m_viewportSize);
		m_renderer2D->onResize(m_viewportSize);

		m_camera.onResize(p_size);
	}

	auto NewLayer::_onKeyPressEvent(tst::KeyPressEvent &p_key_press_event) -> bool
	{
		auto &window{m_app->getWindow()};

		switch (p_key_press_event.getKeyCode())
		{
			case toaster::input::EKeyCode::eEscape:
			{
				m_app->close();
				break;
			}
			case toaster::input::EKeyCode::eF11:
			{
				if (window.isFullscreen())
				{
					window.setWindowed();
					window.maximize();
				}
				else
					window.setFullscreen();
				break;
			}
			default: break;
		}

		return false;
	}

	auto NewLayer::_onMouseScrollEvent(tst::MouseScrollEvent &p_mouse_scroll_event) -> bool
	{
		// m_zoom -= p_mouse_scroll_event.getScrollY() * 0.2f;

		return false;
	}

	auto NewLayer::_onFileDropEvent(tst::WindowFileDropEvent &p_file_drop_event) -> bool
	{
		LOG_INFO("Path: {}", p_file_drop_event.toStr());

		m_image = m_renderCtx->createImageRef(p_file_drop_event.getFilepaths()[0]);

		return true;
	}
}
