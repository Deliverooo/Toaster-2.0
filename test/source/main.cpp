#include "toast_kernel/application.hpp"
#include "toast_kernel/input.hpp"
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
		m_viewportSize  = m_app->getWindow().getRenderAreaSize();
		m_scene         = m_app->createScene("Orbo's Exodus");
		m_sceneRenderer = m_app->createSceneRenderer(m_scene.get());

		gpu::PipelineSpecInfo pipeline_spec_info{};
		pipeline_spec_info.colourAttachments  = {vk::Format::eR8G8B8A8Srgb};
		pipeline_spec_info.shader             = m_globals->shaderLibrary().get("Composite");
		pipeline_spec_info.cullMode           = vk::CullModeFlagBits::eBack;
		pipeline_spec_info.vertexBufferLayout = render::RenderContext::fullscreenQuadVbl;

		auto pipeline{m_renderCtx->createGPURef<gpu::Pipeline>(pipeline_spec_info, "Composite")};

		m_swapchainPass = m_renderCtx->createGPURef<gpu::RenderPass>(pipeline);
		m_swapchainPass->setInput("u_Texture", m_sceneRenderer->getColourTexture()).bake();

		m_scene->setSceneEnvironment(m_renderCtx->createEnvironmentMap("C:/dev/Toaster-2.0/resources/environments/overcast_soil_puresky_2k.hdr"));

		{
			m_quadEntity = m_scene->createEntity("Orbo");
			auto &src{m_quadEntity.addComponent<SpriteRendererComponent>()};
			src.colour = {1.0f, 0.0f, 1.0f, 1.0f};

			auto &mc{m_quadEntity.addComponent<MeshComponent>()};
			mc.mesh = m_renderCtx->createRef<render::MeshData>("C:/dev/Toaster-2.0/resources/meshes/DJT_sculpt.fbx");
		}
		{
		}

		render::Renderer2DSpecInfo renderer_2d_spec_info{};
		renderer_2d_spec_info.renderTargetSize    = m_viewportSize;
		renderer_2d_spec_info.overrideAttachments = true;
		m_renderer2D                              = m_renderCtx->createUnique<render::Renderer2D>(renderer_2d_spec_info);
	}

	auto onUpdate(float32 p_dt) -> void override
	{
		Dx::XMVECTOR delta_position{Dx::XMVectorZero()};

		if (m_inputCtx->isKeyDown(input::EKeyCode::eW))
			delta_position = Dx::XMVectorAdd(delta_position, Dx::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
		if (m_inputCtx->isKeyDown(input::EKeyCode::eS))
			delta_position = Dx::XMVectorAdd(delta_position, Dx::XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f));
		if (m_inputCtx->isKeyDown(input::EKeyCode::eA))
			delta_position = Dx::XMVectorAdd(delta_position, Dx::XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f));
		if (m_inputCtx->isKeyDown(input::EKeyCode::eD))
			delta_position = Dx::XMVectorAdd(delta_position, Dx::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f));

		Dx::XMVECTOR position{Dx::XMLoadFloat3(&m_quadEntity.getTranslation())};
		delta_position = Dx::XMVector3NormalizeSafe(delta_position);

		constexpr float32 speed{5.0f};

		Dx::XMStoreFloat3(&m_quadEntity.getTranslation(), Dx::XMVectorAdd(Dx::XMVectorScale(delta_position, p_dt * speed), position));

		auto       rendering_info{m_app->getWindow().getSwapchainRenderingInfo({1.0f, 1.0f, 1.0f, 1.0f}, false)};
		const auto cmd{m_renderCtx->getCurrentSwapchainCommandBuffer()};

		Dx::XMMATRIX projection{Dx::XMMatrixPerspectiveFovLH(Dx::XMConvertToRadians(90.0f), m_viewportSize.aspect(), 0.1f, 1000.0f)};
		Dx::XMMATRIX view{
			Dx::XMMatrixLookAtLH(Dx::XMVectorSet(0.0f, 0.0f, 3.0f, 0.0f), Dx::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f), Dx::XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f))
		};

		m_scene->onUpdate(p_dt);
		m_sceneRenderer->onRender(view, projection);

		m_renderer2D->begin(view, projection);

		m_renderer2D->submitQuad(Dx::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f), Dx::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f), {1.0f});

		auto colour_attachment_info{m_renderCtx->getRenderingAttachmentInfo(*m_sceneRenderer->getColourTexture()->getImage(), gpu::EAttachmentUsageOP::eLoadStore)};
		auto depth_attachment_info{m_renderCtx->getRenderingAttachmentInfo(*m_sceneRenderer->getDepthTexture()->getImage(), gpu::EAttachmentUsageOP::eLoadStore)};
		m_renderer2D->end(cmd, &colour_attachment_info, &depth_attachment_info);

		m_renderCtx->beginRendering(cmd, rendering_info, m_swapchainPass);
		m_renderCtx->renderFullscreenQuad(cmd, m_swapchainPass, nullptr);
		m_renderCtx->endRendering(cmd, rendering_info);
	}

	auto onResize(tsm::uint2 p_size) -> void override
	{
		m_viewportSize = p_size;

		m_scene->onResize(p_size);
		m_sceneRenderer->onResize(p_size);
		m_renderer2D->onResize(p_size);
	}

private:
	tsm::uint2 m_viewportSize{0u};

	gpu::RenderPassHandle m_swapchainPass{nullptr};

	UniquePtr<Scene>              m_scene{nullptr};
	UniquePtr<SceneRenderer>      m_sceneRenderer{nullptr};
	UniquePtr<render::Renderer2D> m_renderer2D{nullptr};

	Entity m_quadEntity;

	// Dx::XMFLOAT3 m_quadPosition{0.0f, 0.0f, 0.0f};
};

auto main(int32 p_argc, char **p_argv) -> int32
{
	ApplicationSpecInfo app_spec{};
	app_spec.printGPUDebugInfo             = false;
	app_spec.windowSpecInfo.startMaximized = true;
	Application app{app_spec, nullptr};

	app.addLayer<ClientLayer>();

	app.run();
	return 0;
}
