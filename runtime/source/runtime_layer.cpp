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
#include "toast_scene/scriptable_entity.hpp"

namespace toaster
{
	auto RuntimeLayer::onInit() -> void
	{
		io::filesystem::Path binary_dir{os::getBinaryDirectory()};

		m_scene = m_app->createScene("Runtime Scene Test");
		m_scene->setSceneEnvironment(m_renderCtx->createEnvironmentMap(binary_dir / "../resources/environments/grasslands_sunset_1k.hdr"));
		m_sceneRenderer = m_app->createSceneRenderer(m_scene.get());

		gpu::PipelineSpecInfo fullscreen_pipeline_spec_info{};
		fullscreen_pipeline_spec_info.colourAttachments  = {vk::Format::eR8G8B8A8Srgb};
		fullscreen_pipeline_spec_info.shader             = m_globals->shaderLibrary().get("Composite");
		fullscreen_pipeline_spec_info.vertexBufferLayout = render::RenderContext::fullscreenQuadVbl;

		m_fullscreenPipeline   = m_renderCtx->createGPURef<gpu::Pipeline>(fullscreen_pipeline_spec_info);
		m_fullscreenRenderPass = m_renderCtx->createRef<render::RenderPass>(m_fullscreenPipeline);
		m_fullscreenRenderPass->setInput("u_Texture", m_sceneRenderer->getColourTexture()).bake();

		{
			Entity e{m_scene->createEntity("Peeb")};
			e.addComponent<SpriteRendererComponent>();
		}

		{
			Entity e{m_scene->createEntity("Skib")};
			e.addComponent<MeshComponent>().mesh = m_renderCtx->createRef<render::MeshData>(binary_dir / "../resources/meshes/DJT_sculpt.fbx");
			auto &tc{e.getComponent<TransformComponent>()};
		}

		{
			m_cameraEntity = m_scene->createEntity("Player");
			FirstPersonCameraEntityCreateParams params{m_inputCtx, 90.0f, m_viewportSize.aspect(), 0.1f, 1000.0f};
			m_cameraEntity.addComponent<NativeScriptComponent>().bind<FirstPersonCameraEntity>(&params);
			m_scene->initNativeScripts(); // I have to call ts
		}
	}

	auto RuntimeLayer::onUpdate(const float32 p_dt) -> void
	{
		m_scene->onUpdate(p_dt);
		m_sceneRenderer->onRender();

		const auto rendering_info{m_app->getWindow().getSwapchainRenderingInfo({1.0f, 1.0f, 1.0f, 1.0f}, false)};
		const auto cmd{m_renderCtx->getCurrentSwapchainCommandBuffer()};

		m_renderCtx->beginRendering(cmd, rendering_info, m_fullscreenRenderPass);
		m_renderCtx->renderFullscreenQuad(cmd, m_fullscreenRenderPass, nullptr);
		m_renderCtx->endRendering(cmd, rendering_info);
	}

	auto RuntimeLayer::onResize(tsm::uint2 p_size) -> void
	{
		m_viewportSize = p_size;

		m_scene->onResize(p_size);
		m_sceneRenderer->onResize(p_size);
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
