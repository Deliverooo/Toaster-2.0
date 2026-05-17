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
#include "toast_scene/entity.hpp"
#include "toast_scene/scene_importer.hpp"
#include "toast_scene/scene_serializer.hpp"

namespace toaster
{
	RuntimeLayer::RuntimeLayer(Application *p_app) : IAppLayer(p_app)
	{
	}

	auto RuntimeLayer::onInit() -> void
	{
		auto swapchain{m_app->getWindow().getSwapchain()};

		m_viewportWidth  = swapchain->getExtent().width;
		m_viewportHeight = swapchain->getExtent().height;

		swapchain->setResizeCallback([this](const uint32 width, const uint32 height) -> void
		{
			m_viewportWidth  = width;
			m_viewportHeight = height;

			m_scene->setViewportSize(width, height);

			m_sceneRenderer->onResize(width, height);
		});

		auto                 command_line_args{m_app->getCommandLineArgs()};
		io::filesystem::Path binary_dir{os::getBinaryDirectory()};

		#pragma region script + scene setup
		io::filesystem::Path script_asm_path{m_app->getCommandLineArgs()->get("--scriptAsm")};
		io::filesystem::Path core_script_assembly_dll{script_asm_path.parent_path() / "Toaster.dll"};
		LOG_INFO("{}", script_asm_path.string());

		script::ScriptEngineSpecInfo script_engine_spec_info{};
		script_engine_spec_info.rootDomainName   = "ToasterRootDomain";
		script_engine_spec_info.appDomainName    = "ToasterAppDomain";
		script_engine_spec_info.coreAssemblyPath = core_script_assembly_dll;
		script_engine_spec_info.appAssemblyPath  = script_asm_path;
		m_scriptEngine                           = make_unique<script::ScriptEngine>(script_engine_spec_info);

		m_scene = make_reference<Scene>(m_renderCtx, m_scriptEngine.get(), "New Scene");
		m_inputCtx->registerScriptMethods(m_scriptEngine.get());
		#pragma endregion

		auto                  fullscreen_shader{m_globals->shaderLibrary().get("Composite")};
		gpu::PipelineSpecInfo fullscreen_pipeline_spec_info{};
		fullscreen_pipeline_spec_info.colourAttachments  = {swapchain->getSurfaceFormat().format};
		fullscreen_pipeline_spec_info.shader             = fullscreen_shader;
		fullscreen_pipeline_spec_info.cullMode           = vk::CullModeFlagBits::eBack;
		fullscreen_pipeline_spec_info.vertexBufferLayout = gpu::BufferLayout{
			{gpu::EBufferDataType::eFloat3, "a_Position"},
			{gpu::EBufferDataType::eFloat2, "a_TexCoord"}
		};
		m_fullscreenPipeline   = m_renderCtx->createGPU<gpu::VKPipeline>(fullscreen_pipeline_spec_info);
		m_fullscreenRenderPass = m_renderCtx->createGPU<gpu::VKRenderPass>(m_fullscreenPipeline);
		m_fullscreenRenderPass->bake();

		// Ts is not used yet. It would be used for post-processing effects.
		m_fullscreenMaterial = m_renderCtx->create<render::Material>(m_globals->shaderLibrary().get("Composite"));

		SceneRendererSpecInfo scene_renderer_spec_info{};
		scene_renderer_spec_info.viewportWidth     = m_viewportWidth;
		scene_renderer_spec_info.viewportHeight    = m_viewportHeight;
		scene_renderer_spec_info.scene             = m_scene;
		scene_renderer_spec_info.resourceDirectory = binary_dir / "../resources";
		m_sceneRenderer                            = make_reference<SceneRenderer>(m_renderCtx, scene_renderer_spec_info);

		SceneSerializer scene_serializer{m_scene, binary_dir};
		String          scene_path{command_line_args->get("--scene")};
		if (scene_path == "__NONE__")
			scene_serializer.deserialize(binary_dir / "../resources/scenes/Startup_blank.tscene");
		else
			scene_serializer.deserialize(scene_path);

		// SceneImporter scene_importer{m_scene};
		// scene_importer.importFromFile(binary_dir / "../resources/meshes/Test_scene.fbx");
	}

	auto RuntimeLayer::onDestroy() -> void
	{
		m_renderCtx->gpuWaitIdle();
	}

	auto RuntimeLayer::onUpdate(const float32 p_dt) -> void
	{
		auto         swapchain = m_app->getWindow().getSwapchain();
		const uint32 frame_index{swapchain->getFrameIndex()};
		auto &       command_buffer = swapchain->getCurrentCommandBuffer();

		m_scene->onUpdate(p_dt);
		m_scene->onRender(&command_buffer, frame_index, p_dt, m_sceneRenderer);

		m_fullscreenRenderPass->setInput("u_Texture", m_sceneRenderer->getFinalColourTexture());

		gpu::RenderingInfo rendering_info{};
		rendering_info.renderArea = vk::Rect2D{{0, 0}, {m_viewportWidth, m_viewportHeight}};

		auto &colour_attachment_info{rendering_info.colourAttachments.emplace_back()};
		colour_attachment_info.imageView   = swapchain->getCurrentImageView();
		colour_attachment_info.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
		colour_attachment_info.clearValue  = vk::ClearColorValue{1.0f, 1.0f, 1.0f, 1.0f};

		m_renderCtx->beginRendering(&command_buffer, rendering_info, frame_index, m_fullscreenRenderPass);
		m_renderCtx->renderFullscreenQuad(&command_buffer, frame_index, m_fullscreenPipeline, m_fullscreenMaterial);
		m_renderCtx->endRendering(&command_buffer, rendering_info);
	}

	auto RuntimeLayer::onEvent(Event &p_event) -> void
	{
		EventDispatcher eventDispatcher(p_event);
		eventDispatcher.dispatch<KeyPressEvent>(TST_BIND_EVENT_FN(RuntimeLayer::_onKeyPressEvent));

		m_scene->onEvent(p_event);
	}

	auto RuntimeLayer::_onKeyPressEvent(KeyPressEvent &e) -> bool
	{
		auto &window{m_app->getWindow()};
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
