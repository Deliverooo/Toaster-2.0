#include "runtime_layer.hpp"

#include "toaster/toast_kernel/application.hpp"
#include "toaster/toast_kernel/input.hpp"
#include "toaster/toast_lib/io/file_stream.hpp"
#include "toaster/toast_render/globals.hpp"

#include "toast_gpu/vk/vk_swapchain.hpp"

#include "toast_gpu/vk/vk_logical_device.hpp"
#include "toast_lib/os/terminal.hpp"
#include "toast_render/render_context.hpp"
#include "toast_scene/components.hpp"
#include "toast_scene/scene_serializer.hpp"

namespace toaster
{
	RuntimeLayer::RuntimeLayer(Application *p_app) : IAppLayer(p_app), m_cameraTest(p_app->getWindow().getInputContext(), 90.0f, 1.777f, 0.1f, 100.0f)
	{
	}

	auto RuntimeLayer::onInit() -> void
	{
		auto &app = getApp();
		auto  input_ctx{app.getWindow().getInputContext()};

		auto swapchain{app.getWindow().getSwapchain()};
		m_viewportWidth  = swapchain->getExtent().width;
		m_viewportHeight = swapchain->getExtent().height;
		m_cameraTest.setViewportSize(m_viewportWidth, m_viewportHeight);

		swapchain->setResizeCallback([this](const uint32 width, const uint32 height) -> void
		{
			m_viewportWidth  = width;
			m_viewportHeight = height;

			m_scene->setViewportSize(width, height);

			m_sceneRenderer->onResize(width, height);
			m_cameraTest.setViewportSize(width, height);
		});

		auto                 command_line_args{app.getCommandLineArgs()};
		io::filesystem::Path binary_dir{os::getBinaryDirectory()};

		LOG_INFO("Binary directory: {}", binary_dir.string());
		#pragma region script + scene setup

		io::filesystem::Path script_asm_path{app.getCommandLineArgs()->get("--scriptAsm")};

		io::filesystem::Path        core_script_assembly_dll{script_asm_path.parent_path() / "Toaster.dll"};
		const io::filesystem::Path &app_script_assembly_dll{script_asm_path};

		LOG_INFO("{}", app_script_assembly_dll.string());

		script::ScriptEngineSpecInfo script_engine_spec_info{};
		script_engine_spec_info.rootDomainName   = "ToasterRootDomain";
		script_engine_spec_info.appDomainName    = "ToasterAppDomain";
		script_engine_spec_info.coreAssemblyPath = core_script_assembly_dll;
		script_engine_spec_info.appAssemblyPath  = app_script_assembly_dll;
		m_scriptEngine                           = make_unique<script::ScriptEngine>(script_engine_spec_info);

		m_scene = make_reference<Scene>(m_renderCtx, m_scriptEngine.get(), "New Scene");
		input_ctx->registerScriptMethods(m_scriptEngine.get());
		#pragma endregion

		auto                  fullscreen_shader{m_renderCtx->getGlobals()->shaderLibrary().get("Composite")};
		gpu::PipelineSpecInfo fullscreen_pipeline_spec_info{};
		fullscreen_pipeline_spec_info.colourAttachments  = {swapchain->getSurfaceFormat().format};
		fullscreen_pipeline_spec_info.shader             = fullscreen_shader;
		fullscreen_pipeline_spec_info.depthCompare       = vk::CompareOp::eAlways;
		fullscreen_pipeline_spec_info.cullMode           = vk::CullModeFlagBits::eBack; // We don't want to cull our viewport
		fullscreen_pipeline_spec_info.vertexBufferLayout = gpu::BufferLayout{
			{gpu::EBufferDataType::eFloat3, "a_Position"},
			{gpu::EBufferDataType::eFloat2, "a_TexCoord"}
		};
		m_fullscreenPipeline   = m_renderCtx->createGPU<gpu::VKPipeline>(fullscreen_pipeline_spec_info);
		m_fullscreenRenderPass = m_renderCtx->createGPU<gpu::VKRenderPass>(m_fullscreenPipeline);
		m_fullscreenRenderPass->bake();

		m_fullscreenMaterial = make_reference<render::Material>(m_renderCtx, m_renderCtx->getGlobals()->shaderLibrary().get("Composite"));

		SceneRendererSpecInfo scene_renderer_spec_info{};
		scene_renderer_spec_info.viewportWidth     = m_viewportWidth;
		scene_renderer_spec_info.viewportHeight    = m_viewportHeight;
		scene_renderer_spec_info.scene             = m_scene.get();
		scene_renderer_spec_info.resourceDirectory = binary_dir / "../resources";
		m_sceneRenderer                            = make_reference<SceneRenderer>(m_renderCtx, scene_renderer_spec_info);

		SceneSerializer scene_serializer{m_scene, binary_dir};
		String          scene_path{command_line_args->get("--scene")};
		if (scene_path == "__NONE__")
			scene_serializer.deserialize(binary_dir / "../resources/scenes/Test.tscene");
		else
			scene_serializer.deserialize(scene_path);

		m_testTex = m_renderCtx->createGPU<gpu::VKTexture2D>(gpu::TextureSpecInfo{}, binary_dir / "../resources/textures/Peeber.png");
	}

	auto RuntimeLayer::onDestroy() -> void
	{
		m_renderCtx->gpuWaitIdle();
	}

	auto RuntimeLayer::onUpdate(const float32 p_dt) -> void
	{
		auto &app       = getApp();
		auto  swapchain = app.getWindow().getSwapchain();

		uint32 frame_index{swapchain->getFrameIndex()};
		auto & command_buffer = swapchain->getCurrentCommandBuffer();

		m_cameraTest.onUpdate(p_dt);
		m_scene->onUpdate(p_dt);
		m_scene->onRender(command_buffer, frame_index, p_dt, m_sceneRenderer);

		auto tex{m_sceneRenderer->getOutputColourTexture()};
		// if (m_inputCtx->isKeyPressed(input::EKeyCode::eP))
		// {
		// tex->saveToFile(os::getBinaryDirectory() / "Orbo.png");
		// }

		gpu::util::transitionImageLayout(tex->getImage().get(), tex->getImage()->getCurrentImageLayout(), vk::ImageLayout::eShaderReadOnlyOptimal);

		m_fullscreenMaterial->setTexture("u_Texture", tex);

		gpu::RenderingInfo rendering_info{};
		rendering_info.renderArea = vk::Rect2D{{0, 0}, {m_viewportWidth, m_viewportHeight}};

		auto &colour_attachment_info{rendering_info.colourAttachments.emplace_back()};
		colour_attachment_info.imageView   = swapchain->getCurrentImageView();
		colour_attachment_info.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
		colour_attachment_info.clearValue  = vk::ClearColorValue{1.0f, 1.0f, 1.0f, 1.0f};

		m_renderCtx->beginRendering(command_buffer, rendering_info, frame_index, m_fullscreenRenderPass);
		m_renderCtx->renderFullscreenQuad(command_buffer, frame_index, m_fullscreenPipeline, m_fullscreenMaterial);
		m_renderCtx->endRendering(command_buffer, rendering_info);
	}

	auto RuntimeLayer::onEvent(Event &p_event) -> void
	{
		EventDispatcher eventDispatcher(p_event);
		eventDispatcher.dispatch<KeyPressEvent>(TST_BIND_EVENT_FN(RuntimeLayer::_onKeyPressEvent));

		m_scene->onEvent(p_event);
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
			{
				window.setWindowed();
			}
		}

		return false;
	}
}
