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
	ClientLayer::ClientLayer(Application *p_app) : IAppLayer(p_app), m_editorCamera(90.0f, 1.777, 0.1f, 100.0f)
	{
	}

	void ClientLayer::onInit()
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

			m_MSAAColourAttachmentImage->resize(width, height);
			m_MSAADepthAttachmentImage->resize(width, height);

			m_renderer2D->onResize(width, height);
			m_sceneRenderer->onResize(width, height);
			m_editorCamera.setViewportSize(width, height);
		});

		{
			gpu::ImageCreateInfo colour_attachment_image_create_info{};
			colour_attachment_image_create_info.width       = window_width;
			colour_attachment_image_create_info.height      = window_height;
			colour_attachment_image_create_info.format      = swapchain->getSurfaceFormat().format;
			colour_attachment_image_create_info.usage       = vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment;
			colour_attachment_image_create_info.sampleCount = ctx->getMaxUsableSampleCount();
			m_MSAAColourAttachmentImage                     = make_reference<gpu::VKImage2D>(ctx, colour_attachment_image_create_info);

			gpu::ImageCreateInfo depth_attachment_image_create_info{};
			depth_attachment_image_create_info.width       = window_width;
			depth_attachment_image_create_info.height      = window_height;
			depth_attachment_image_create_info.format      = swapchain->getDepthFormat();
			depth_attachment_image_create_info.usage       = vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eDepthStencilAttachment;
			depth_attachment_image_create_info.sampleCount = ctx->getMaxUsableSampleCount();
			m_MSAADepthAttachmentImage                     = make_reference<gpu::VKImage2D>(ctx, depth_attachment_image_create_info);
		}
		{
			auto                    composite_shader{Globals::getShaderLibrary().get("Composite")};
			gpu::PipelineCreateInfo pipeline_create_info{};
			pipeline_create_info.vertexBufferLayout = {{gpu::EShaderDataType::eFloat3, "a_Position"}, {gpu::EShaderDataType::eFloat2, "a_TexCoord"}};
			pipeline_create_info.colourAttachments  = {swapchain->getSurfaceFormat().format};
			pipeline_create_info.depthFormat        = {swapchain->getDepthFormat()};
			pipeline_create_info.shader             = composite_shader;
			m_compositePipeline                     = make_reference<gpu::VKPipeline>(ctx, pipeline_create_info);

			m_fullscreenPass = make_reference<gpu::VKRenderPass>(ctx, m_compositePipeline);
			m_fullscreenPass->bake();

			m_fullscreenMaterial = make_reference<gpu::VKMaterial>(ctx, composite_shader);
		}

		auto geometry_shader{Globals::getShaderLibrary().get("Geometry")};
		m_mesh  = make_reference<gpu::VKMesh>(ctx, "../resources/meshes/Orbo.fbx", geometry_shader);
		m_mesh2 = make_reference<gpu::VKMesh>(ctx, "../resources/meshes/DJT_sculpt.fbx", geometry_shader);

		Renderer2DCreateInfo renderer_2d_create_info{};
		renderer_2d_create_info.renderTargetWidth  = window_width;
		renderer_2d_create_info.renderTargetHeight = window_height;
		m_renderer2D                               = make_reference<Renderer2D>(ctx, renderer_2d_create_info);

		SceneRendererSpecInfo scene_renderer_spec_info{};
		scene_renderer_spec_info.viewportWidth  = m_viewportWidth;
		scene_renderer_spec_info.viewportHeight = m_viewportHeight;
		scene_renderer_spec_info.scene          = nullptr;
		m_sceneRenderer                         = make_reference<SceneRenderer>(ctx, scene_renderer_spec_info);
	}

	void ClientLayer::onDestroy()
	{
		auto &app = getApp();
		auto  ctx = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());
		ctx->getDevice().waitIdle();
	}

	void ClientLayer::onUpdate(const float32 p_dt)
	{
		m_time += p_dt;

		auto &app       = getApp();
		auto  ctx       = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());
		auto  swapchain = app.getWindow().getSwapchain();

		uint32 frame_index{swapchain->getFrameIndex()};
		uint32 image_index{swapchain->getImageIndex()};

		vk::Extent2D swapchain_extent{swapchain->getExtent()};
		auto &       command_buffer = swapchain->getCurrentCommandBuffer();

		m_editorCamera.onUpdate(p_dt);

		CameraUB camera_ub{};
		camera_ub.view = m_editorCamera.getViewMatrix();
		camera_ub.proj = m_editorCamera.getProjectionMatrix();
		// camera_ub.view = glm::lookAt(glm::vec3{2.0f, 2.0f, 2.0f}, glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 0.0f, 1.0f});
		// camera_ub.proj = glm::perspective(glm::radians(45.0f), static_cast<float32>(swapchain_extent.width) / static_cast<float32>(swapchain_extent.height), 0.1f, 10.0f);
		// camera_ub.proj[1][1] *= -1.0f;

		// m_renderer2D->begin(command_buffer, frame_index, camera_ub.view, camera_ub.proj);
		// m_renderer2D->submitQuad({0.0f, 0.0f}, glm::vec2{10.0f, 10.0f}, glm::vec4{1.0f, 1.0f, 1.0f, 1.0f});
		// m_renderer2D->submitQuad(m_meshTranslation, glm::vec2{10.0f, 10.0f}, glm::vec4{1.0f, 1.0f, 1.0f, 1.0f});
		// m_renderer2D->end(command_buffer, frame_index);

		m_sceneRenderer->begin(command_buffer, frame_index, camera_ub.view, camera_ub.proj);
		m_sceneRenderer->renderMesh(m_mesh, glm::translate(glm::scale(glm::mat4{1.0f}, glm::vec3{10.0f, 10.0f, 10.0f}), m_meshTranslation));
		m_sceneRenderer->end(command_buffer, frame_index);

		{
			m_fullscreenPass->setInput("u_Texture", m_sceneRenderer->getOutputColourTexture());

			gpu::RenderingInfo rendering_info{};
			rendering_info.renderArea = vk::Rect2D{{0, 0}, swapchain_extent};
			rendering_info.layerCount = 1;

			gpu::RenderingAttachmentInfo &colour_attachment_info{rendering_info.colourAttachments.emplace_back()};
			colour_attachment_info.clearValue  = vk::ClearColorValue{0.005f, 0.005f, 0.005f, 1.0f};
			colour_attachment_info.imageView   = swapchain->getImageView(image_index);
			colour_attachment_info.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
			colour_attachment_info.loadOp      = vk::AttachmentLoadOp::eClear;
			colour_attachment_info.storeOp     = vk::AttachmentStoreOp::eStore;

			gpu::RenderingAttachmentInfo depth_attachment_info{};
			depth_attachment_info.clearValue  = vk::ClearDepthStencilValue{1.0f, 0u};
			depth_attachment_info.imageView   = swapchain->getDepthImageView();
			depth_attachment_info.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
			depth_attachment_info.loadOp      = vk::AttachmentLoadOp::eClear;
			depth_attachment_info.storeOp     = vk::AttachmentStoreOp::eStore;
			rendering_info.pDepthAttachment   = &depth_attachment_info;

			Renderer::beginRendering(rendering_info, command_buffer, frame_index, m_fullscreenPass);
			Renderer::renderFullscreenQuad(command_buffer, frame_index, m_fullscreenPass->getPipeline(), m_fullscreenMaterial);
			Renderer::endRendering(rendering_info, command_buffer);
		}
	}

	void ClientLayer::onEvent(Event &p_event)
	{
		EventDispatcher eventDispatcher(p_event);
		eventDispatcher.dispatch<KeyPressEvent>(TST_BIND_EVENT_FN(ClientLayer::onKeyPressEvent));
		eventDispatcher.dispatch<WindowResizeEvent>(TST_BIND_EVENT_FN(ClientLayer::onWindowResizeEvent));
	}

	void ClientLayer::onUIRender()
	{
		ig::Begin("Tools");

		ig::SliderFloat3("Translation", glm::value_ptr(m_meshTranslation), -1.0f, 1.0f);

		ig::End();
	}

	bool ClientLayer::onKeyPressEvent(KeyPressEvent &e)
	{
		if (e.getKeyCode() == input::EKeyCode::eEscape)
			getApp().close();

		return false;
	}

	bool ClientLayer::onWindowResizeEvent(WindowResizeEvent &e)
	{
		return false;
	}
}
