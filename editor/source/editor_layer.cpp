#include "editor_layer.hpp"

#include "toast_gpu/vk/vk_gpu_context.hpp"
#include "toast_gpu/vk/vk_swapchain.hpp"
#include "toast_lib/events/key_event.hpp"
#include "toast_render/globals.hpp"
#include "toast_render/renderer.hpp"

namespace toaster
{
	EditorLayer::EditorLayer(Application *p_app) : IAppLayer(p_app)
	{
	}

	void EditorLayer::onInit()
	{
		const auto &app{getApp()};
		auto        ctx{dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext())};
		const auto  swapchain{app.getWindow().getSwapchain()};

		m_viewportWidth  = swapchain->getExtent().width;
		m_viewportHeight = swapchain->getExtent().height;

		swapchain->addResizeCallback([this](const uint32 width, const uint32 height) -> void
		{
			m_viewportWidth  = width;
			m_viewportHeight = height;
		});

		auto fullscreen_shader{Globals::getShaderLibrary().get("Composite")};

		gpu::PipelineCreateInfo fullscreen_pipeline_create_info{};
		fullscreen_pipeline_create_info.colourAttachments  = {swapchain->getSurfaceFormat().format};
		fullscreen_pipeline_create_info.depthFormat        = swapchain->getDepthFormat();
		fullscreen_pipeline_create_info.shader             = fullscreen_shader;
		fullscreen_pipeline_create_info.cullMode           = vk::CullModeFlagBits::eNone; // We don't want to cull our viewport
		fullscreen_pipeline_create_info.vertexBufferLayout = gpu::VertexBufferLayout{
			{gpu::EShaderDataType::eFloat3, "a_Position"},
			{gpu::EShaderDataType::eFloat2, "a_TexCoord"}
		};

		m_fullscreenPipeline = make_reference<gpu::VKPipeline>(ctx, fullscreen_pipeline_create_info);
		m_fullscreenPass     = make_reference<gpu::VKRenderPass>(ctx, m_fullscreenPipeline);
		m_fullscreenMaterial = make_reference<gpu::VKMaterial>(ctx, fullscreen_shader);

		m_frameDataUBOs = make_reference<gpu::VKUniformBufferPFF>(ctx, sizeof(FrameDataUB), gpu::VKGPUContext::c_maxFramesInFlight);
		m_fullscreenPass->setInput("FrameData", m_frameDataUBOs);

		gpu::TextureSpecInfo texture_spec_info{};
		m_texture = make_reference<gpu::VKTexture2D>(ctx, texture_spec_info, "../resources/textures/Peeber.png");

		m_fullscreenPass->setInput("u_Texture", m_texture);
		m_fullscreenPass->bake();
	}

	void EditorLayer::onDestroy()
	{
	}

	void EditorLayer::onUpdate(float32 p_dt)
	{
		m_time += p_dt;

		const auto &app{getApp()};
		auto        ctx{dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext())};
		const auto  swapchain{app.getWindow().getSwapchain()};

		const auto & cmd_buf{swapchain->getCurrentCommandBuffer()};
		const uint32 frame_index{swapchain->getFrameIndex()};

		FrameDataUB frame_data{};
		frame_data.res.x = static_cast<float32>(m_viewportWidth);
		frame_data.res.y = static_cast<float32>(m_viewportHeight);
		frame_data.time  = m_time;
		m_frameDataUBOs->getUBO(frame_index)->setData(&frame_data, sizeof(FrameDataUB), 0u);

		m_fullscreenMaterial->set("u_Constants.intensity", glm::abs(glm::sin(m_time)));

		gpu::RenderingInfo rendering_info{};
		rendering_info.renderArea = vk::Rect2D{{0, 0}, {m_viewportWidth, m_viewportHeight}};

		auto &colour_attachment_info{rendering_info.colourAttachments.emplace_back()};
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

		rendering_info.pDepthAttachment = &depth_attachment_info;

		Renderer::beginRendering(rendering_info, cmd_buf, frame_index, m_fullscreenPass);
		Renderer::renderFullscreenQuad(cmd_buf, frame_index, m_fullscreenPipeline, m_fullscreenMaterial);
		Renderer::endRendering(rendering_info, cmd_buf);
	}

	void EditorLayer::onEvent(Event &p_event)
	{
		auto &app{getApp()};

		EventDispatcher event_dispatcher{p_event};
		event_dispatcher.dispatch<KeyPressEvent>([&app](const KeyPressEvent &e) -> bool
		{
			switch (e.getKeyCode())
			{
				case input::EKeyCode::eEscape: app.close();
				default: break;
			}
			return false;
		});
	}

	void EditorLayer::onUIRender()
	{
	}
}
