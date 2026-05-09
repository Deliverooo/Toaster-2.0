#include "runtime_layer.hpp"

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
#include "toast_scene/scene_serializer.hpp"
namespace ig = ImGui;

namespace toaster
{
	RuntimeLayer::RuntimeLayer(Application *p_app) : IAppLayer(p_app)
	{
	}

	auto RuntimeLayer::onInit() -> void
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

			m_scene->setViewportSize(width, height);

			m_sceneRenderer->onResize(width, height);
		});

		const auto &binary_dir{app.getExeDirectory()};

		#pragma region script + scene setup

		io::filesystem::Path core_script_assembly_dll{};
		io::filesystem::Path app_script_assembly_dll{};
		if (app.getCommandLineArgs().size() > 1) // Custom app script dll
		{
			// When compiling, the Toaster.dll should automatically be in the same directory as the app one... :)
			core_script_assembly_dll = io::filesystem::Path{app.getCommandLineArgs()[1]}.parent_path() / "Toaster.dll";
			app_script_assembly_dll  = app.getCommandLineArgs()[1];
		}
		else
		{
			core_script_assembly_dll = binary_dir / "../scripts/Toaster/bin/Debug/net48/Toaster.dll";  // Fallback to the prebuild toaster dll
			app_script_assembly_dll  = binary_dir / "../examples/Sandbox/bin/Debug/net48/Sandbox.dll"; // Fallback to the demo script
		}

		LOG_INFO("{}", app_script_assembly_dll.string());

		script::ScriptEngineSpecInfo script_engine_spec_info{};
		script_engine_spec_info.rootDomainName   = "ToasterRootDomain";
		script_engine_spec_info.appDomainName    = "ToasterAppDomain";
		script_engine_spec_info.coreAssemblyPath = core_script_assembly_dll;
		script_engine_spec_info.appAssemblyPath  = app_script_assembly_dll;
		m_scriptEngine                           = make_unique<script::ScriptEngine>(script_engine_spec_info);

		m_scene = make_reference<Scene>(app.getLogicalDevice(), m_scriptEngine.get(), "New Scene");
		input_ctx->registerScriptMethods(m_scriptEngine.get());
		#pragma endregion

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

		SceneRendererSpecInfo scene_renderer_spec_info{};
		scene_renderer_spec_info.viewportWidth     = m_viewportWidth;
		scene_renderer_spec_info.viewportHeight    = m_viewportHeight;
		scene_renderer_spec_info.scene             = m_scene.get();
		scene_renderer_spec_info.resourceDirectory = binary_dir / "../resources";
		m_sceneRenderer                            = toaster::make_reference<SceneRenderer>(device, scene_renderer_spec_info);

		#if 0
		io::filesystem::Path shader_dir{binary_dir / "../source/toaster/toast_shaders"};
		{
			Entity orbo_entity{m_scene->createEntity("Orbo")};
			orbo_entity.addComponent<MeshComponent>().mesh = device->alloc<gpu::VKMesh>(binary_dir / "../resources/meshes/Test_scene.fbx",
																						Globals::getShaderLibrary().get("Geometry"));
			orbo_entity.addComponent<ScriptComponent>().className = "Sandbox.Player";
		}

		{
			Entity camera_controller{m_scene->createEntity("Camera controller")};
			camera_controller.addComponent<ScriptComponent>().className = "Sandbox.CameraController";
		}

		{
			Entity point_light_entity{m_scene->createEntity("Point Light 1")};
			auto & tc{point_light_entity.getComponent<TransformComponent>()};
			auto & plc{point_light_entity.addComponent<PointLightComponent>()};
			tc.translation = {-1.0f, 1.0f, 1.0f};
			plc.radiance   = tsm::colours::magenta;
			plc.multiplier = 3.0f;
		}

		{
			Entity point_light_entity{m_scene->createEntity("Point Light 2")};
			auto & tc{point_light_entity.getComponent<TransformComponent>()};
			auto & plc{point_light_entity.addComponent<PointLightComponent>()};
			tc.translation = {1.0f, 1.0f, 1.0f};
			plc.radiance   = tsm::colours::weezer;
			plc.multiplier = 3.0f;
		}
		#endif

		SceneSerializer scene_serializer{m_scene, binary_dir};
		scene_serializer.deserialize(binary_dir / "../resources/scenes/Test.tscene");

		script::clr::CLRScriptEngineSpecInfo clr_script_engine_spec_info{};
		clr_script_engine_spec_info.coreAssemblyPath = core_script_assembly_dll;
		script::clr::CLRScriptEngine clr_script_engine{clr_script_engine_spec_info};
	}

	auto RuntimeLayer::onDestroy() -> void
	{
		auto &app = getApp();
		auto  device{app.getLogicalDevice()};
		device->getVulkanLogicalDevice().waitIdle();
	}

	auto RuntimeLayer::onUpdate(const float32 p_dt) -> void
	{
		auto &app       = getApp();
		auto  swapchain = app.getWindow().getSwapchain();

		uint32 frame_index{swapchain->getFrameIndex()};
		auto & command_buffer = swapchain->getCurrentCommandBuffer();

		m_scene->onUpdate(p_dt);
		m_scene->onRender(command_buffer, frame_index, p_dt, m_sceneRenderer);

		auto tex{m_sceneRenderer->getOutputColourTexture()};
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

		render::beginRendering(rendering_info, command_buffer, frame_index, m_fullscreenRenderPass);
		render::renderFullscreenQuad(command_buffer, frame_index, m_fullscreenPipeline, m_fullscreenMaterial);
		render::endRendering(rendering_info, command_buffer);
	}

	auto RuntimeLayer::onEvent(Event &p_event) -> void
	{
		EventDispatcher eventDispatcher(p_event);
		eventDispatcher.dispatch<KeyPressEvent>(TST_BIND_EVENT_FN(RuntimeLayer::_onKeyPressEvent));
	}

	auto RuntimeLayer::_onKeyPressEvent(KeyPressEvent &e) -> bool
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
