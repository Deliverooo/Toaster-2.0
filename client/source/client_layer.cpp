#include "client_layer.hpp"

#include "stb/stb_image.h"
#include "toaster/toast_kernel/application.hpp"
#include "toaster/toast_kernel/input.hpp"
#include "toaster/toast_lib/io/file_stream.hpp"
#include "toaster/toast_render/globals.hpp"
#include "toaster/toast_render/renderer.hpp"

#include "toast_lib/logging.hpp"

#include "toast_gpu/vk/vk_swapchain.hpp"

#include <imgui.h>

#include "glm/gtc/type_ptr.hpp"
namespace ig = ImGui;

namespace toaster
{
	ClientLayer::ClientLayer(Application *p_app) : IAppLayer(p_app)
	{
	}

	auto ClientLayer::onInit() -> void
	{
		auto &app       = getApp();
		auto  ctx       = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());
		auto  swapchain = app.getWindow().getSwapchain();

		uint32 window_width{swapchain->getExtent().width};
		uint32 window_height{swapchain->getExtent().height};

		m_viewportWidth  = window_width;
		m_viewportHeight = window_height;

		swapchain->addResizeCallback([this](uint32 width, uint32 height)
		{
			LOG_INFO("{}, {}", width, height);

			m_viewportWidth  = width;
			m_viewportHeight = height;

			m_renderer2D->onResize(width, height);
		});

		Renderer2DCreateInfo renderer_2d_create_info{};
		renderer_2d_create_info.renderTargetWidth  = window_width;
		renderer_2d_create_info.renderTargetHeight = window_height;
		m_renderer2D                               = make_reference<Renderer2D>(ctx, renderer_2d_create_info);
	}

	auto ClientLayer::onDestroy() -> void
	{
		auto &app = getApp();
		auto  ctx = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());
		ctx->getDevice().waitIdle();
	}

	auto ClientLayer::onUpdate(const float32 p_dt) -> void
	{
		m_time += p_dt;

		auto &app       = getApp();
		auto  ctx       = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());
		auto  swapchain = app.getWindow().getSwapchain();

		uint32 frame_index{swapchain->getFrameIndex()};
		uint32 image_index{swapchain->getImageIndex()};

		vk::Extent2D swapchain_extent{swapchain->getExtent()};
		auto &       command_buffer = swapchain->getCurrentCommandBuffer();

		m_renderer2D->begin(command_buffer, frame_index, glm::mat4{1.0f}, glm::mat4{1.0f});

		gpu::RenderingAttachmentInfo colour_attachment_info{};
		colour_attachment_info.imageView   = swapchain->getCurrentImageView();
		colour_attachment_info.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
		colour_attachment_info.loadOp      = vk::AttachmentLoadOp::eClear;
		colour_attachment_info.storeOp     = vk::AttachmentStoreOp::eStore;
		colour_attachment_info.clearValue  = vk::ClearColorValue{1.0f, 1.0f, 1.0f, 1.0f};

		gpu::RenderingAttachmentInfo depth_attachment_info{};
		depth_attachment_info.imageView   = swapchain->getDepthImageView();
		depth_attachment_info.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
		depth_attachment_info.loadOp      = vk::AttachmentLoadOp::eClear;
		depth_attachment_info.storeOp     = vk::AttachmentStoreOp::eStore;
		depth_attachment_info.clearValue  = vk::ClearDepthStencilValue{1.0f, 0u};
		m_renderer2D->end(command_buffer, frame_index, &colour_attachment_info, &depth_attachment_info);
	}

	auto ClientLayer::onEvent(Event &p_event) -> void
	{
		EventDispatcher eventDispatcher(p_event);
		eventDispatcher.dispatch<KeyPressEvent>(TST_BIND_EVENT_FN(ClientLayer::_onKeyPressEvent));
		eventDispatcher.dispatch<WindowResizeEvent>(TST_BIND_EVENT_FN(ClientLayer::_onWindowResizeEvent));
	}

	auto ClientLayer::onUIRender() -> void
	{
	}

	auto ClientLayer::_onKeyPressEvent(KeyPressEvent &e) -> bool
	{
		if (e.getKeyCode() == input::EKeyCode::eEscape)
			getApp().close();

		return false;
	}

	auto ClientLayer::_onWindowResizeEvent(WindowResizeEvent &e) -> bool
	{
		return false;
	}
}
