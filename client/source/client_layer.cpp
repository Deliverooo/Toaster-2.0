#include "client_layer.hpp"

#include "toaster/toast_kernel/application.hpp"
#include "toaster/toast_kernel/input.hpp"
#include "toaster/toast_lib/io/file_stream.hpp"
#include "toaster/toast_render/globals.hpp"
#include "toaster/toast_render/renderer.hpp"

#include "toast_lib/logging.hpp"

#include "toast_gpu/vk/vk_swapchain.hpp"

namespace toaster
{
	ClientLayer::ClientLayer(Application *p_app) : IAppLayer(p_app)
	{
	}

	void ClientLayer::onInit()
	{
		auto &app = getApp();
		auto  ctx = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());

		auto physical_device = ctx->getPhysicalDevice();
		auto props           = physical_device.getProperties();
		LOG_INFO("Name: {}", props.deviceName.data());
		LOG_INFO("Type: {}", vk::to_string(props.deviceType));

		auto swapchain = app.getWindow().getSwapchain();

		gpu::PipelineCreateInfo pipeline_create_info{};
		pipeline_create_info.colourAttachmentFormat = swapchain->getSurfaceFormat().format;
		m_pipeline                                  = make_reference<gpu::VKPipeline>(ctx, pipeline_create_info);
	}

	void ClientLayer::onDestroy()
	{
	}

	void ClientLayer::onUpdate(const float32 p_dt)
	{
		auto &app       = getApp();
		auto  ctx       = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());
		auto  swapchain = app.getWindow().getSwapchain();

		_recordCommandBuffer(swapchain->getImageIndex());
	}

	void ClientLayer::onEvent(Event &p_event)
	{
		EventDispatcher eventDispatcher(p_event);
		eventDispatcher.dispatch<KeyPressEvent>(TST_BIND_EVENT_FN(ClientLayer::onKeyPressEvent));
	}

	bool ClientLayer::onKeyPressEvent(KeyPressEvent &e)
	{
		if (e.getKeyCode() == input::EKeyCode::eEscape)
		{
			getApp().close();
		}

		return false;
	}

	void ClientLayer::_recordCommandBuffer(uint32 p_image_index)
	{
		auto &app       = getApp();
		auto  ctx       = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());
		auto  swapchain = app.getWindow().getSwapchain();

		auto &command_buffer = swapchain->getCurrentCommandBuffer();

		vk::ClearValue              clear_value = vk::ClearColorValue{0.0f, 0.0f, 0.0f, 1.0f};
		vk::RenderingAttachmentInfo rendering_attachment_info{};
		rendering_attachment_info.clearValue  = clear_value;
		rendering_attachment_info.imageView   = swapchain->getImageView(p_image_index);
		rendering_attachment_info.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
		rendering_attachment_info.loadOp      = vk::AttachmentLoadOp::eClear;
		rendering_attachment_info.storeOp     = vk::AttachmentStoreOp::eStore;

		vk::RenderingInfo rendering_info{};
		rendering_info.renderArea           = vk::Rect2D{{0, 0}, swapchain->getExtent()};
		rendering_info.layerCount           = 1;
		rendering_info.colorAttachmentCount = 1;
		rendering_info.pColorAttachments    = &rendering_attachment_info;

		vk::Viewport viewport{};
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		viewport.x        = 0.0f;
		viewport.y        = 0.0f;
		viewport.width    = static_cast<float32>(swapchain->getExtent().width);
		viewport.height   = static_cast<float32>(swapchain->getExtent().height);

		vk::Rect2D scissor{};
		scissor.offset = vk::Offset2D{0, 0};
		scissor.extent = swapchain->getExtent();

		command_buffer.beginRendering(rendering_info);
		command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_pipeline->getPipeline());
		command_buffer.setViewport(0, viewport);
		command_buffer.setScissor(0, scissor);
		command_buffer.draw(3, 1, 0, 0);
		command_buffer.endRendering();
	}
}
