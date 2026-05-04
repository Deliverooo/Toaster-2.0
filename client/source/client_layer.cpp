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
#include "toast_gpu/vk/vk_logical_device.hpp"
#include "toast_gpu/vk/vk_shader_compiler.hpp"
#include "toast_scene/components.hpp"
#include "toast_scene/entity.hpp"
namespace ig = ImGui;

namespace toaster
{
	ClientLayer::ClientLayer(Application *p_app) : IAppLayer(p_app)
	{
	}

	auto ClientLayer::onInit() -> void
	{
		auto &app    = getApp();
		auto  device = app.getLogicalDevice();
		auto  input_ctx{app.getWindow().getInputContext()};

		auto   swapchain{app.getWindow().getSwapchain()};
		uint32 window_width{swapchain->getExtent().width};
		uint32 window_height{swapchain->getExtent().height};

		#pragma region script + scene setup
		io::filesystem::Path scripts_dir{"../scripts"};

		script::ScriptEngineSpecInfo script_engine_spec_info{};
		script_engine_spec_info.rootDomainName = "ToasterRootDomain";
		script_engine_spec_info.appDomainName  = "ToasterAppDomain";
		script_engine_spec_info.assemblyPath   = scripts_dir / "Toaster/bin/Debug/net48/Toaster.dll";
		m_scriptEngine                         = make_unique<script::ScriptEngine>(script_engine_spec_info);

		m_scene = make_reference<Scene>(app.getLogicalDevice(), m_scriptEngine.get(), "New Scene");
		input_ctx->registerScriptMethods(m_scriptEngine.get());
		#pragma endregion

		m_camera = EditorCamera{input_ctx, 90.0f, static_cast<float32>(window_width) / static_cast<float32>(window_height), 0.1f, 1000.0f};

		swapchain->setResizeCallback([&](const uint32 width, const uint32 height) -> void
		{
			window_width  = width;
			window_height = height;

			m_scene->setViewportSize(width, height);

			m_sceneRenderer->onResize(width, height);
			m_camera.setViewportSize(static_cast<float32>(width), static_cast<float32>(height));
		});

		auto                    fullscreen_shader{Globals::getShaderLibrary().get("Composite")};
		gpu::PipelineCreateInfo fullscreen_pipeline_create_info{};
		fullscreen_pipeline_create_info.colourAttachments  = {swapchain->getSurfaceFormat().format};
		fullscreen_pipeline_create_info.depthFormat        = swapchain->getDepthFormat();
		fullscreen_pipeline_create_info.shader             = fullscreen_shader;
		fullscreen_pipeline_create_info.cullMode           = vk::CullModeFlagBits::eNone; // We don't want to cull our viewport
		fullscreen_pipeline_create_info.vertexBufferLayout = gpu::BufferLayout{
			{gpu::EBufferDataType::eFloat3, "a_Position"},
			{gpu::EBufferDataType::eFloat2, "a_TexCoord"}
		};
		m_fullscreenPipeline   = device->alloc<gpu::VKPipeline>(fullscreen_pipeline_create_info);
		m_fullscreenRenderPass = device->alloc<gpu::VKRenderPass>(m_fullscreenPipeline);
		m_fullscreenRenderPass->bake();

		SceneRendererSpecInfo scene_renderer_spec_info{};
		scene_renderer_spec_info.viewportWidth  = window_width;
		scene_renderer_spec_info.viewportHeight = window_height;
		scene_renderer_spec_info.scene          = m_scene.get();
		m_sceneRenderer                         = toaster::make_reference<SceneRenderer>(device, scene_renderer_spec_info);

		io::filesystem::Path shader_dir{"../source/toaster/toast_shaders"};
		m_shaderLibrary.add("Mesh Test", gpu::VKShaderCompiler::compileToShaderFromPaths(device, {vk::ShaderStageFlagBits::eVertex, vk::ShaderStageFlagBits::eFragment},
																						 {shader_dir / "mesh.vert.glsl", shader_dir / "mesh.pixel.glsl"}));

		{
			Entity orbo_entity{m_scene->createEntity("Orbo")};
			orbo_entity.addComponent<MeshComponent>().mesh = device->alloc<gpu::VKMesh>("../resources/meshes/Orbo.fbx", Globals::getShaderLibrary().get("Geometry"));
			orbo_entity.addComponent<ScriptComponent>().className = "Toaster.Player";
		}

		{
			Entity camera_controller{m_scene->createEntity("Camera controller")};
			camera_controller.addComponent<ScriptComponent>().className = "Toaster.CameraController";
		}
	}

	auto ClientLayer::onDestroy() -> void
	{
		auto &app = getApp();
		auto  device{app.getLogicalDevice()};
		device->getVulkanLogicalDevice().waitIdle();
	}

	auto ClientLayer::onUpdate(const float32 p_dt) -> void
	{
		m_time += p_dt;

		auto &app       = getApp();
		auto  swapchain = app.getWindow().getSwapchain();

		uint32 frame_index{swapchain->getFrameIndex()};

		vk::Extent2D swapchain_extent{swapchain->getExtent()};
		auto &       command_buffer = swapchain->getCurrentCommandBuffer();

		m_camera.onUpdate(p_dt);

		m_scene->onUpdate(p_dt);
		// m_scene->onRender(command_buffer, frame_index, p_dt, m_sceneRenderer, m_camera.getViewMatrix(), m_camera.getProjectionMatrix());
		m_scene->onRender(command_buffer, frame_index, p_dt, m_sceneRenderer);

		m_fullscreenRenderPass->setInput("u_Texture", m_sceneRenderer->getOutputColourTexture());

		gpu::RenderingInfo rendering_info{};
		rendering_info.renderArea = vk::Rect2D{{0, 0}, swapchain_extent};

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

		render::beginRendering(rendering_info, command_buffer, frame_index, m_fullscreenRenderPass);
		render::renderFullscreenQuad(command_buffer, frame_index, m_fullscreenPipeline, nullptr);
		render::endRendering(rendering_info, command_buffer);
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

	auto ClientLayer::_onWindowResizeEvent(WindowResizeEvent &e) -> bool
	{
		return false;
	}
}
