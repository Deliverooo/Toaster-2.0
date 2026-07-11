#include <toast_kernel/application.hpp>
#include <toast_kernel/fp_camera.hpp>
#include <toast_kernel/layer.hpp>

#include <toast_render/globals.hpp>
#include <toast_render/render_context.hpp>

#include <toast_lib/os/terminal.hpp>

#include "toast_kernel/input.hpp"
#include "toast_render/dynamic_mesh.hpp"
#include "toast_render/skybox_pass.hpp"

#include "toast_render/dynamic_material.hpp"
#include "toast_scene/scene_renderer.hpp"

using namespace toaster;

class ClientLayer : public IAppLayer
{
public:
	auto onInit() -> void override
	{
		m_viewportSize = m_app->getWindow().getRenderAreaSize();

		m_camera = FPCamera{m_inputCtx, 90.0f, m_viewportSize.aspect(), 0.1f, 1000.0f};

		const auto binary_dir{os::getBinaryDirectory()};
		const auto resources_dir{binary_dir / "../resources"};

		m_scene = toaster::make_unique<scene::Scene>(m_renderCtx);

		m_scene->setSkyboxMap(m_renderCtx->createEnvironmentMapImage(resources_dir / "environments/qwantani_dusk_2_puresky_2k.hdr"));
		m_sceneRenderer = toaster::make_unique<scene::SceneRenderer>(*m_scene, m_viewportSize);

		auto orbo_geo{m_renderCtx->createRef<render::DynamicMesh>(resources_dir / "meshes/Orbo_Geo.fbx")};
		orbo_geo->getMaterial(0)->set("metalness", 0.0f);
		orbo_geo->getMaterial(1)->set("metalness", 1.0f);

		auto djt_geo{m_renderCtx->createRef<render::DynamicMesh>(resources_dir / "meshes/DJT_sculpt.fbx")};

		scene::Entity test_scene_entity{};
		{
			test_scene_entity = m_scene->createEntity("Test_Scene");
			auto &mesh_comp{test_scene_entity.addComponent<scene::DynamicMeshComponent>()};
			mesh_comp.mesh = orbo_geo;

			class LightController : public scene::ScriptableEntity
			{
			public:
				auto onCreate(void *p_user_data) -> void override
				{
					m_inputCtx = static_cast<InputContext *>(p_user_data);

					LOG_INFO("{}", scene->getName());
				}

				auto onUpdate(float32 p_dt) -> void override
				{
					Dx::XMVECTOR translation{Dx::XMLoadFloat3(&transform->translation)};

					if (m_inputCtx->isKeyDown(input::EKeyCode::eUp))
						translation = Dx::XMVectorAdd(translation, Dx::XMVectorSet(0.0f, p_dt, 0.0f, 0.0f));
					if (m_inputCtx->isKeyDown(input::EKeyCode::eDown))
						translation = Dx::XMVectorAdd(translation, Dx::XMVectorSet(0.0f, -p_dt, 0.0f, 0.0f));
					if (m_inputCtx->isKeyDown(input::EKeyCode::eRight))
						translation = Dx::XMVectorAdd(translation, Dx::XMVectorSet(p_dt, 0.0f, 0.0f, 0.0f));
					if (m_inputCtx->isKeyDown(input::EKeyCode::eLeft))
						translation = Dx::XMVectorAdd(translation, Dx::XMVectorSet(-p_dt, 0.0f, 0.0f, 0.0f));

					Dx::XMStoreFloat3(&transform->translation, translation);
				}

			private:
				NonOwningPtr<InputContext> m_inputCtx{nullptr};
			};

			test_scene_entity.addComponent<scene::NativeScriptComponent>().bind<LightController>(m_inputCtx);
		}

		{
			scene::Entity other_entity{m_scene->createEntity("Other")};
			other_entity.addComponent<scene::DynamicMeshComponent>().mesh = djt_geo;
			other_entity.setParent(test_scene_entity);
		}

		for (uint32 i{0u}; i < 10; ++i)
		{
			for (uint32 j{0u}; j < 10; ++j)
			{
				scene::Entity entity{m_scene->createEntity(fmt::format("Entity_{}", i * 10 + j))};
				entity.addComponent<scene::DynamicMeshComponent>().mesh = orbo_geo;

				auto &tc{entity.getComponent<scene::TransformComponent>()};
				tc.translation = {(float32) i * 2.0f, 0.0f, (float32) j * 2.0f};
			}
		}

		{
			scene::Entity light_entity{m_scene->createEntity("Light_001")};
			auto &        plc{light_entity.addComponent<scene::PointLightComponent>()};
			plc.radiance   = {1.0f, 1.0f, 1.0f};
			plc.multiplier = 10.0f;

			auto &tc{light_entity.getComponent<scene::TransformComponent>()};
			tc.translation = {0.0f, 5.0f, 1.0f};
		}

		m_scene->initNativeScripts();

		auto features{gpu::VKLogicalDeviceSpecInfo::getDefaultFeatures()};
		LOG_INFO("Features struct size: {}", sizeof(decltype(features)));
	}

	auto onDestroy() -> void override
	{
	}

	auto onUpdate(float32 p_dt) -> void override
	{
		static float32 time{0.0f};
		time += p_dt;

		m_camera.onUpdate(p_dt);
		m_scene->onUpdate(p_dt);

		m_sceneRenderer->onRender(m_camera.getPosition(), m_camera.getViewMatrix(), m_camera.getProjectionMatrix());

		auto       rendering_info{m_app->getWindow().getSwapchainRenderingInfo(false, {0.025f, 0.025f, 0.025f, 1.0f}, false)};
		const auto cmd{m_renderCtx->getCurrentCommandBuffer()};

		cmd->bindShaders({m_globals->getShader("Fullscreen_Quad_VS"), m_globals->getShader("Fullscreen_Quad_PS")});
		cmd->setPrimitiveTopology(gpu::EPrimitiveTopology::eTriangleList);
		cmd->setPrimitiveRestartEnable(false);
		cmd->setDepthClampEnable(false);
		cmd->setDepthBiasEnable(false);
		cmd->setRasterizerDiscardEnable(false);
		cmd->setPolygonMode(gpu::EPolygonMode::eFill);
		cmd->setCullMode(gpu::ECullMode::eNone);
		cmd->setFrontFace(gpu::EFrontFace::eCCW);
		cmd->setLineWidth(1.0f);
		cmd->setColourBlendEnable({false});
		cmd->setColourWriteMask({vk::FlagTraits<vk::ColorComponentFlagBits>::allFlags});
		cmd->setDepthTestEnable(false);
		cmd->setStencilTestEnable(false);
		cmd->setRasterizationSamples(gpu::ESampleCount::e1);
		cmd->setAlphaToCoverageEnable(false);

		m_renderCtx->beginRendering(rendering_info);
		cmd->pushData<FullscreenQuadConstants>({
												   m_globals->fullscreenQuadVertexBuffer().getDeviceAddress(),
												   m_renderCtx->getSampler(render::ESamplerType::eDefault),
												   m_sceneRenderer->getColourImage()->getAlignedShaderReadHeapID()
											   });
		m_renderCtx->renderFullscreenQuad();
		m_renderCtx->endRendering(rendering_info);
	}

	auto onResize(tsm::uint2 p_size) -> void override
	{
		m_viewportSize = p_size;

		m_scene->onResize(m_viewportSize);
		m_sceneRenderer->onResize(m_viewportSize);

		m_camera.onResize(p_size);
	}

	auto onEvent(Event &p_event) -> void override
	{
		m_camera.onEvent(p_event);
	}

private:
	tsm::uint2 m_viewportSize{0u};

	FPCamera m_camera{};

	TST_PUSH_CONSTANT_BLOCK(FullscreenQuadConstants)
	{
		uintptr vertexBufferBDA;

		uint32 sampler;
		uint32 texture;
	};

	UniquePtr<scene::Scene>         m_scene{nullptr};
	UniquePtr<scene::SceneRenderer> m_sceneRenderer{nullptr};
};

auto main(int32 p_argc, char **p_argv) -> int32
{
	ApplicationSpecInfo app_spec{};
	app_spec.printGPUDebugInfo             = true;
	app_spec.windowSpecInfo.startMaximized = true;
	Application app{app_spec, nullptr};

	app.addLayer<ClientLayer>();

	try
	{
		app.run();
	}
	catch (const std::exception &e)
	{
		TST_PERMA_ASSERT_MSG(false, fmt::format("Exception: {}", e.what()).c_str());
		return -1;
	}

	return 0;
}
