#include "launcher_layer.hpp"

#include "toaster/toast_kernel/application.hpp"
#include "toaster/toast_kernel/input.hpp"
#include "toaster/toast_lib/io/file_stream.hpp"
#include "toaster/toast_render/globals.hpp"
#include "toaster/toast_render/renderer.hpp"

#include "toast_gpu/vk/vk_swapchain.hpp"

#include "glm/gtc/type_ptr.hpp"
#include "toast_gpu/vk/vk_logical_device.hpp"
#include "toast_gpu/vk/vk_renderer.hpp"
#include "toast_lib/os/terminal.hpp"
#include "toast_scene/components.hpp"
#include "toast_scene/scene_serializer.hpp"

namespace toaster
{
	LauncherLayer::LauncherLayer(Application *p_app) : IAppLayer(p_app)
	{
	}

	auto LauncherLayer::onInit() -> void
	{
		auto &app    = getApp();
		auto  device = app.getLogicalDevice();
		auto  input_ctx{app.getWindow().getInputContext()};

		auto swapchain{app.getWindow().getSwapchain()};
		m_viewportWidth  = swapchain->getExtent().width;
		m_viewportHeight = swapchain->getExtent().height;

		swapchain->setResizeCallback([this](const uint32 width, const uint32 height) -> void
		{
			m_viewportWidth  = width;
			m_viewportHeight = height;

			m_renderer2D->onResize(width, height);
		});

		auto                 command_line_args{app.getCommandLineArgs()};
		io::filesystem::Path binary_dir{os::getBinaryDirectory()};
		LOG_INFO("Binary directory: {}", binary_dir.string());

		auto                  fullscreen_shader{Globals::getShaderLibrary().get("Composite")};
		gpu::PipelineSpecInfo fullscreen_pipeline_spec_info{};
		fullscreen_pipeline_spec_info.colourAttachments  = {swapchain->getSurfaceFormat().format};
		fullscreen_pipeline_spec_info.depthFormat        = swapchain->getDepthFormat();
		fullscreen_pipeline_spec_info.shader             = fullscreen_shader;
		fullscreen_pipeline_spec_info.cullMode           = vk::CullModeFlagBits::eNone; // We don't want to cull our viewport
		fullscreen_pipeline_spec_info.vertexBufferLayout = gpu::BufferLayout{
			{gpu::EBufferDataType::eFloat3, "a_Position"},
			{gpu::EBufferDataType::eFloat2, "a_TexCoord"}
		};
		m_fullscreenPipeline   = device->alloc<gpu::VKPipeline>(fullscreen_pipeline_spec_info);
		m_fullscreenRenderPass = device->alloc<gpu::VKRenderPass>(m_fullscreenPipeline);
		m_fullscreenRenderPass->bake();
		m_fullscreenMaterial = device->alloc<gpu::VKMaterial>(Globals::getShaderLibrary().get("Composite"));

		Renderer2DSpecInfo renderer_2d_spec_info{};
		renderer_2d_spec_info.renderTargetWidth  = m_viewportWidth;
		renderer_2d_spec_info.renderTargetHeight = m_viewportHeight;
		m_renderer2D                             = make_unique<Renderer2D>(device, renderer_2d_spec_info);
	}

	auto LauncherLayer::onDestroy() -> void
	{
		auto &app = getApp();
		auto  device{app.getLogicalDevice()};
		device->getVulkanLogicalDevice().waitIdle();
	}

	auto LauncherLayer::onUpdate(const float32 p_dt) -> void
	{
		auto &app       = getApp();
		auto  swapchain = app.getWindow().getSwapchain();

		uint32 frame_index{swapchain->getFrameIndex()};
		auto & command_buffer = swapchain->getCurrentCommandBuffer();

		glm::mat4 view{1.0f};
		glm::mat4 proj{glm::ortho(-1.0f, 1.0f, swapchain->getAspectRatio(), -swapchain->getAspectRatio())};
		m_renderer2D->begin(command_buffer, frame_index, view, proj);
		m_renderer2D->submitQuad({0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f});
		m_renderer2D->end(command_buffer, frame_index);

		auto tex{m_renderer2D->getOutputColourTexture()};
		m_fullscreenRenderPass->setInput("u_Texture", tex);

		m_fullscreenMaterial->set("u_Constants.res", glm::vec2{m_viewportWidth, m_viewportHeight});

		gpu::RenderingInfo rendering_info{};
		rendering_info.renderArea = vk::Rect2D{{0, 0}, {m_viewportWidth, m_viewportHeight}};

		auto &colour_attachment_info{rendering_info.colourAttachments.emplace_back()};
		colour_attachment_info.imageView   = swapchain->getCurrentImageView();
		colour_attachment_info.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
		colour_attachment_info.loadOp      = vk::AttachmentLoadOp::eClear;
		colour_attachment_info.storeOp     = vk::AttachmentStoreOp::eStore;
		colour_attachment_info.clearValue  = vk::ClearColorValue{0.0f, 1.0f, 1.0f, 1.0f};

		gpu::RenderingAttachmentInfo depth_attachment_info{};
		depth_attachment_info.imageView   = swapchain->getDepthImageView();
		depth_attachment_info.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
		depth_attachment_info.loadOp      = vk::AttachmentLoadOp::eClear;
		depth_attachment_info.storeOp     = vk::AttachmentStoreOp::eStore;
		depth_attachment_info.clearValue  = vk::ClearDepthStencilValue{1.0f, 0u};
		rendering_info.pDepthAttachment   = std::addressof(depth_attachment_info);

		gpu::render::beginRendering(rendering_info, command_buffer, frame_index, m_fullscreenRenderPass);
		render::renderFullscreenQuad(command_buffer, frame_index, m_fullscreenPipeline, m_fullscreenMaterial);
		gpu::render::endRendering(rendering_info, command_buffer);
	}

	auto LauncherLayer::onEvent(Event &p_event) -> void
	{
		EventDispatcher eventDispatcher(p_event);
		eventDispatcher.dispatch<KeyPressEvent>(TST_BIND_EVENT_FN(LauncherLayer::_onKeyPressEvent));
	}

	auto LauncherLayer::_onKeyPressEvent(KeyPressEvent &e) -> bool
	{
		auto &app{getApp()};
		auto &window{app.getWindow()};

		if (e.getKeyCode() == input::EKeyCode::eF11)
		{
			if (!window.isFullscreen())
				window.setFullscreen();
			else
				window.setWindowed();
		}

		return false;
	}
}
