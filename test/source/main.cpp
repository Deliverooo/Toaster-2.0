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
#include "toast_scene/dynamic_scene_renderer.hpp"

using namespace toaster;

class ClientLayer : public IAppLayer
{
public:
	struct SceneDataUB
	{
		Dx::XMFLOAT3 cameraPos;
		float32      _padd[1];
	};

	auto onInit() -> void override
	{
		m_viewportSize = m_app->getWindow().getRenderAreaSize();

		m_camera = FPCamera{m_inputCtx, 90.0f, m_viewportSize.aspect(), 0.1f, 1000.0f};

		const auto binary_dir{os::getBinaryDirectory()};
		const auto resources_dir{binary_dir / "../resources"};

		m_environmentMap       = m_renderCtx->createEnvironmentMapImage(resources_dir / "environments/overcast_soil_puresky_2k.hdr");
		m_diffuseIrradianceMap = m_renderCtx->createDiffuseIrradianceMapImage(m_environmentMap);

		m_scene = toaster::make_unique<Scene>(m_renderCtx);
		m_scene->setSceneEnvironmentImage(m_environmentMap);
		m_sceneRenderer = toaster::make_unique<DynamicSceneRenderer>(m_scene.get(), m_viewportSize);

		const auto vertex_shader{m_globals->getShader("Fullscreen_Quad_VS")};
		const auto pixel_shader{m_globals->getShader("Fullscreen_Quad_PS")};

		m_fullscreenState = m_renderCtx->createUnique<render::GraphicsState>();
		m_fullscreenState->setShaders({vertex_shader, pixel_shader}).setAttachmentCount(1u).setCullMode(vk::CullModeFlagBits::eNone).setEnableDepthTest(false).
				setEnableDepthWrite(false).setVertexBufferLayout(render::RenderContext::fullscreenQuadVbl);

		{
			Entity orbo_entity{m_scene->createEntity("Orbo")};
			auto & mesh_comp{orbo_entity.addComponent<DynamicMeshComponent>()};
			mesh_comp.mesh = m_renderCtx->createRef<render::DynamicMesh>(resources_dir / "meshes/Backrooms.fbx");
		}

		{
			Entity teapot_entity{m_scene->createEntity("Teapot")};
			auto & mesh_comp{teapot_entity.addComponent<DynamicMeshComponent>()};
			mesh_comp.mesh = m_renderCtx->createRef<render::DynamicMesh>(resources_dir / "meshes/Orbo_Geo.fbx");

			mesh_comp.mesh->getMaterial(0)->set("metalness", 0.0f);
			mesh_comp.mesh->getMaterial(1)->set("metalness", 1.0f);
		}
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
		const auto cmd{m_renderCtx->getCurrentSwapchainCommandBuffer()};

		m_fullscreenState->bind();

		m_renderCtx->beginRendering(rendering_info);
		FullscreenQuadConstants quad_constants{};
		quad_constants.sampler = m_renderCtx->getSampler(render::ESamplerType::eDefault);
		quad_constants.texture = m_sceneRenderer->getColourImage()->getAlignedShaderReadHeapID();
		cmd->pushData(quad_constants);
		m_renderCtx->renderFullscreenQuad();
		m_renderCtx->endRendering(rendering_info);
	}

	auto onResize(tsm::uint2 p_size) -> void override
	{
		m_viewportSize = p_size;

		m_scene->onResize(m_viewportSize);
		m_sceneRenderer->onResize(m_viewportSize);

		// m_MSAAColour->resize(m_viewportSize);
		// m_MSAADepth->resize(m_viewportSize);

		m_camera.onResize(p_size);
		// m_skyboxPass->onResize(p_size);
	}

	auto onEvent(Event &p_event) -> void override
	{
		m_camera.onEvent(p_event);
	}

private:
	tsm::uint2 m_viewportSize{0u};

	FPCamera m_camera{};
	// gpu::RawImageUnique m_MSAAColour{nullptr};
	// gpu::RawImageUnique m_MSAADepth{nullptr};

	// render::ImageHandle m_image{nullptr};

	render::GraphicsStateUnique m_fullscreenState{nullptr};

	TST_PUSH_CONSTANT_BLOCK(FullscreenQuadConstants)
	{
		uint32 sampler;
		uint32 texture;
	};

	UniquePtr<Scene>                m_scene{nullptr};
	UniquePtr<DynamicSceneRenderer> m_sceneRenderer{nullptr};

	// render::UniformBufferPFFUnique m_cameraUBOs{nullptr};
	// render::UniformBufferPFFUnique m_sceneDataUBOs{nullptr};

	render::ImageHandle m_environmentMap{nullptr};
	render::ImageHandle m_diffuseIrradianceMap{nullptr};

	// render::GraphicsStateUnique m_geometryGraphicsState{nullptr};

	#if 0
	TST_PUSH_CONSTANT_BLOCK(MeshletMeshConstants)
	{
		Dx::XMFLOAT4X4 meshTransform;

		uintptr vertexBuffer;

		uintptr material;

		uintptr cameraPtr;
		uintptr sceneDataPtr;

		uint32 samplerIndex;
		uint32 diffuseIrradianceMapIndex;
	}; UniquePtr<render::SkyboxPass> m_skyboxPass{nullptr};
	#endif
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
	catch (const vk::DeviceLostError &e)
	{
		LOG_FATAL("Device lost error: {}", e.what());

		return -1;
	}

	return 0;
}
