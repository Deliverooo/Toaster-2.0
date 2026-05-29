#include "runtime_layer.hpp"

#include "toast_kernel/input.hpp"
#include "toast_lib/io/file_stream.hpp"
#include "toast_render/globals.hpp"

#include "toast_gpu/vk/vk_swapchain.hpp"

#include "toast_gpu/vk/vk_logical_device.hpp"
#include "toast_kernel/application.hpp"
#include "toast_lib/os/terminal.hpp"
#include "toast_render/render_context.hpp"
#include "toast_scene/components.hpp"
#include "toast_scene/entity.hpp"
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

		io::filesystem::Path binary_dir{os::getBinaryDirectory()};

		m_scene = make_reference<Scene>(m_renderCtx, nullptr, "Orbo's Exodus");
		m_inputCtx->registerScriptMethods(nullptr);

		#pragma region render stuff setup
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

		SceneRendererSpecInfo scene_renderer_spec_info{};
		scene_renderer_spec_info.viewportWidth  = m_viewportWidth;
		scene_renderer_spec_info.viewportHeight = m_viewportHeight;
		scene_renderer_spec_info.scene          = m_scene;
		m_sceneRenderer                         = make_reference<SceneRenderer>(m_renderCtx, scene_renderer_spec_info);
		#pragma endregion

		m_scene->setSceneEnvironment(m_renderCtx->createEnvironmentMap("C:/dev/Toaster-2.0/resources/environments/grasslands_sunset_1k.hdr"));

		{
			Entity e{m_scene->createEntity("Orbo")};
			auto & cam{e.addComponent<CameraComponent>()};
			cam.primary = true;
			cam.camera.setPerspectiveFov(glm::radians(90.0f));
			cam.camera.setProjectionType(SceneCamera::EProjectionType::ePerspective);
		}
	}

	auto RuntimeLayer::onDestroy() -> void
	{
		m_renderCtx->gpuWaitIdle();
	}

	auto RuntimeLayer::onUpdate(const float32 p_dt) -> void
	{
		auto  swapchain      = m_app->getWindow().getSwapchain();
		auto &command_buffer = swapchain->getCurrentCommandBuffer();

		m_scene->onUpdate(p_dt);
		m_scene->onRender(&command_buffer, p_dt, m_sceneRenderer);

		m_fullscreenRenderPass->setInput("u_Texture", m_sceneRenderer->getResolveOutputColourTexture());

		gpu::RenderingInfo rendering_info{};
		rendering_info.renderArea = vk::Rect2D{{0, 0}, {m_viewportWidth, m_viewportHeight}};

		auto &colour_attachment_info{rendering_info.colourAttachments.emplace_back()};
		colour_attachment_info.imageView   = swapchain->getCurrentImageView();
		colour_attachment_info.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
		colour_attachment_info.clearValue  = vk::ClearColorValue{1.0f, 1.0f, 1.0f, 1.0f};

		m_renderCtx->beginRendering(&command_buffer, rendering_info, m_fullscreenRenderPass);
		m_renderCtx->renderFullscreenQuad(&command_buffer, m_fullscreenPipeline, nullptr);
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
