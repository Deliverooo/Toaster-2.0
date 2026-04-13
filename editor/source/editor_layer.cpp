#include "editor_layer.hpp"

#include "toast_gpu/vk/vk_gpu_context.hpp"
#include "toast_gpu/vk/vk_swapchain.hpp"
#include "toast_lib/events/key_event.hpp"
#include "toast_render/globals.hpp"
#include "toast_render/renderer.hpp"
#include "toast_scene/components.hpp"
#include "toast_scene/entity.hpp"
#include "toast_scene/scene_renderer.hpp"
#include "toast_kernel/input.hpp"

#include <imgui.h>

#include "toast_lib/os/file_dialog.hpp"
namespace ig = ImGui;

namespace toaster
{
	EditorLayer::EditorLayer(Application *p_app) : IAppLayer(p_app), m_editorCamera(90.0f, 1.777f, 0.1f, 100.0f)
	{
	}

	void EditorLayer::onInit()
	{
		const auto &app{getApp()};
		auto        ctx{dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext())};
		const auto  swapchain{app.getWindow().getSwapchain()};

		m_viewportWidth  = swapchain->getExtent().width;
		m_viewportHeight = swapchain->getExtent().height;

		m_editorCamera.setViewportSize(static_cast<float32>(m_viewportWidth), static_cast<float32>(m_viewportHeight));

		swapchain->addResizeCallback([this](const uint32 width, const uint32 height) -> void
		{
			m_viewportWidth  = width;
			m_viewportHeight = height;

			m_sceneRenderer->onResize(width, height);
			m_editorCamera.setViewportSize(static_cast<float32>(width), static_cast<float32>(height));
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
		m_fullscreenPipeline = ctx->alloc<gpu::VKPipeline>(fullscreen_pipeline_create_info);
		m_fullscreenPass     = ctx->alloc<gpu::VKRenderPass>(m_fullscreenPipeline);
		m_fullscreenMaterial = ctx->alloc<gpu::VKMaterial>(fullscreen_shader);

		m_frameDataUBOs = ctx->alloc<gpu::VKUniformBufferPFF>(sizeof(FrameDataUB), gpu::VKGPUContext::c_maxFramesInFlight);
		m_fullscreenPass->setInput("FrameData", m_frameDataUBOs);

		gpu::TextureSpecInfo texture_spec_info{};
		m_texture = ctx->alloc<gpu::VKTexture2D>(texture_spec_info, "../resources/textures/Peeber.png");
		gpu::TextureSpecInfo texture_spec_info2{};
		m_texture2 = ctx->alloc<gpu::VKTexture2D>(texture_spec_info2, "../resources/textures/ooorbo.png");

		m_fullscreenPass->setInput("u_Texture", m_texture);
		m_fullscreenPass->bake();

		m_scene = make_reference<Scene>(ctx, "Main Scene");

		m_sceneHierarchyPanel = make_unique<SceneHierarchyPanel>(ctx, m_scene);

		SceneRendererSpecInfo scene_renderer_spec_info{};
		scene_renderer_spec_info.viewportWidth  = m_viewportWidth;
		scene_renderer_spec_info.viewportHeight = m_viewportHeight;
		scene_renderer_spec_info.scene          = m_scene;
		m_sceneRenderer                         = make_reference<SceneRenderer>(ctx, scene_renderer_spec_info);

		Renderer2DCreateInfo renderer_2d_create_info{};
		renderer_2d_create_info.renderTargetWidth  = m_viewportWidth;
		renderer_2d_create_info.renderTargetHeight = m_viewportHeight;
		m_renderer2D                               = make_reference<Renderer2D>(ctx, renderer_2d_create_info);

		{
			Entity orbo_entity{m_scene->createEntity()};
			auto & transform_comp{orbo_entity.getComponent<TransformComponent>()};
			transform_comp.scale       = {100.0f, 100.0f, 100.0f};
			transform_comp.translation = {0.0f, 0.0f, 0.0f};
			auto &mc{orbo_entity.addComponent<MeshComponent>()};
			mc.mesh = ctx->alloc<gpu::VKMesh>("../resources/meshes/Orbo.fbx", Globals::getShaderLibrary().get("Geometry"));
		}
	}

	void EditorLayer::onDestroy()
	{
	}

	void EditorLayer::onUpdate(const float32 p_dt)
	{
		m_time += p_dt;

		if (m_viewportFocused)
			m_editorCamera.onUpdate(p_dt);

		const auto &app{getApp()};
		auto        ctx{dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext())};
		const auto  swapchain{app.getWindow().getSwapchain()};

		const auto & cmd_buf{swapchain->getCurrentCommandBuffer()};
		const uint32 frame_index{swapchain->getFrameIndex()};

		m_scene->onUpdate(p_dt);
		m_scene->onRender(cmd_buf, frame_index, p_dt, m_sceneRenderer, m_editorCamera.getViewMatrix(), m_editorCamera.getProjectionMatrix());

		m_fullscreenPass->setInput("u_Texture", m_sceneRenderer->getOutputColourTexture());

		{
			FrameDataUB frame_data{};
			frame_data.res.x = static_cast<float32>(m_viewportWidth);
			frame_data.res.y = static_cast<float32>(m_viewportHeight);
			frame_data.time  = m_time;
			m_frameDataUBOs->getUBO(frame_index)->setData(&frame_data, sizeof(FrameDataUB), 0u);

			m_fullscreenMaterial->set("u_Constants.intensity", glm::abs(glm::sin(m_time)));
		}

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
		m_editorCamera.onEvent(p_event);
	}

	void EditorLayer::onUIRender()
	{
		const auto &app{getApp()};
		auto        ctx{dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext())};

		ig::Begin("Properties");

		if (ig::IsWindowFocused() || ig::IsWindowHovered())
			m_viewportFocused = false;
		else
			m_viewportFocused = true;

		ig::End();

		m_sceneHierarchyPanel->onUIRender();

		ig::Begin("Renderer settings");

		ig::Text("Scene renderer background");
		if (ig::Button("File", ImVec2{ig::GetContentRegionAvail().x, 0}))
		{
			auto path = os::openFileDialog({{"Texture", "png,jpg,bmp"}});
			if (io::filesystem::exists(path))
			{
				LOG_INFO("{}", path.string());

				gpu::TextureSpecInfo texture_spec_info{};
				m_sceneRenderer->setEnvironmentBackground(ctx->alloc<gpu::VKTexture2D>(texture_spec_info, path));
			}
		}

		ig::End();
	}
}
