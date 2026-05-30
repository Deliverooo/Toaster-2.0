#include "toast_kernel/application.hpp"
#include "toast_render/globals.hpp"
#include "toast_render/render_context.hpp"
#include "toast_scene/entity.hpp"
#include "toast_scene/scene.hpp"
#include "toast_scene/scene_renderer.hpp"

using namespace toaster;

class ClientLayer : public IAppLayer
{
public:
	auto onInit() -> void override
	{
		m_scene         = m_app->createScene("Orbo's Exodus");
		m_sceneRenderer = m_app->createSceneRenderer(m_scene.get());

		gpu::PipelineSpecInfo pipeline_spec_info{};
		pipeline_spec_info.colourAttachments  = {vk::Format::eR8G8B8A8Srgb};
		pipeline_spec_info.shader             = m_globals->shaderLibrary().get("Composite");
		pipeline_spec_info.cullMode           = vk::CullModeFlagBits::eBack;
		pipeline_spec_info.vertexBufferLayout = render::RenderContext::fullscreenQuadVbl;

		auto pipeline{m_renderCtx->createGPU<gpu::Pipeline>(pipeline_spec_info)};
		m_swapchainPass = m_renderCtx->createGPU<gpu::RenderPass>(pipeline);
		m_swapchainPass->setInput("u_Texture", m_sceneRenderer->getResolveOutputColourTexture()).bake();

		m_scene->setSceneEnvironment(m_renderCtx->createEnvironmentMap("C:/dev/Toaster-2.0/resources/environments/overcast_soil_puresky_2k.hdr"));
		{
			Entity e{m_scene->createEntity("Orbo")};
			auto & cam{e.addComponent<CameraComponent>()};
			cam.camera.setProjectionType(SceneCamera::EProjectionType::ePerspective);
			cam.camera.setPerspectiveFov(tsm::radians(90.0f));
			cam.primary = true;
		}
	}

	auto onUpdate(float32 p_dt) -> void override
	{
		m_scene->onUpdate(p_dt);
		m_sceneRenderer->onRender();

		const auto rendering_info{m_app->getWindow().getSwapchainRenderingInfo({1.0f, 1.0f, 1.0f, 1.0f}, false)};
		const auto cmd{m_renderCtx->getCurrentSwapchainCommandBuffer()};

		m_renderCtx->beginRendering(cmd, rendering_info, m_swapchainPass);
		m_renderCtx->renderFullscreenQuad(cmd, m_swapchainPass, nullptr);
		m_renderCtx->endRendering(cmd, rendering_info);
	}

	auto onResize(tsm::uint2 p_size) -> void override
	{
		m_viewportSize = p_size;
		m_scene->onResize(p_size);
		m_sceneRenderer->onResize(p_size);
	}

private:
	tsm::uint2 m_viewportSize{0u};

	gpu::RenderPassHandle m_swapchainPass{nullptr};

	UniquePtr<Scene>         m_scene{nullptr};
	UniquePtr<SceneRenderer> m_sceneRenderer{nullptr};
};

auto main(int32 p_argc, char **p_argv) -> int32
{
	tsm::float4x4 m{1.0f};
	m = tsm::translate(m, {1.0f, 1.0f, 1.0f});

	LOG_INFO("{}", m);

	ApplicationSpecInfo app_spec{};
	app_spec.printGPUDebugInfo             = false;
	app_spec.windowSpecInfo.startMaximized = true;
	Application app{app_spec, nullptr};

	app.addLayer<ClientLayer>();

	app.run();
	return 0;
}
