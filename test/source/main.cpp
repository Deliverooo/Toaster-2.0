#include "toast_gpu/vk/vk_buffer.hpp"
#include "toast_gpu/vk/vk_descriptor_heap.hpp"
#include "toast_gpu/vk/vk_shader_compiler.hpp"
#include "toast_kernel/application.hpp"
#include "toast_kernel/input.hpp"
#include "toast_lib/os/file_dialog.hpp"
#include "toast_lib/os/terminal.hpp"
#include "toast_render/globals.hpp"
#include "toast_render/graphics_state.hpp"
#include "toast_render/image.hpp"
#include "toast_render/render_context.hpp"
#include "toast_scene/entity.hpp"
#include "toast_scene/scene.hpp"
#include "toast_scene/dynamic_scene_renderer.hpp"

#include "toast_kernel/fp_camera.hpp"
#include "toast_math/colours.hpp"

#include <stb/stb_image.h>

using namespace toaster;

class ClientLayer : public IAppLayer
{
public:
	struct CameraUB
	{
		Dx::XMFLOAT4X4 view;
		Dx::XMFLOAT4X4 proj;
		Dx::XMFLOAT4X4 invProj;
	};

	auto onInit() -> void override
	{
		m_viewportSize = m_app->getWindow().getRenderAreaSize();

		m_camera = FPCamera{m_inputCtx, 90.0f, m_viewportSize.aspect(), 0.1f, 1000.0f};

		auto binary_dir{os::getBinaryDirectory()};
		auto resources_dir{binary_dir / "../resources"};

		m_image = m_renderCtx->createImageRef(resources_dir / "textures/Peeber.png");

		m_graphicsState = m_renderCtx->createUnique<render::GraphicsState>();
		m_graphicsState->setShaders({m_globals->dynamicShaderLibrary().get("Fullscreen_Quad_VS"), m_globals->dynamicShaderLibrary().get("Fullscreen_Quad_PS")}).
				setVertexBufferLayout(render::RenderContext::fullscreenQuadVbl).setAttachmentCount(1u).setCullMode(vk::CullModeFlagBits::eNone).setEnableDepthTest(false).
				setEnableDepthWrite(false);

		// m_mesh = m_renderCtx->createRef<render::DynamicMesh>(resources_dir / "meshes/Tall_Orange_Mike.fbx");
		m_mesh = m_renderCtx->createRef<render::DynamicMesh>(resources_dir / "meshes/Test_scene.fbx");

		m_scene         = m_app->createScene();
		m_sceneRenderer = toaster::make_unique<DynamicSceneRenderer>(m_scene.get(), m_viewportSize);

		const io::filesystem::Path environment_map_path{resources_dir / "environments/grasslands_sunset_1k.hdr"};
		m_scene->setSceneEnvironmentImage(m_renderCtx->createEnvironmentMapImage(environment_map_path));

		{
			Entity e{m_scene->createEntity("Mesh thing")};
			e.addComponent<DynamicMeshComponent>().mesh = m_mesh;
		}
	}

	auto onDestroy() -> void override
	{
	}

	auto onUpdate(float32 p_dt) -> void override
	{
		auto rendering_info{m_app->getWindow().getSwapchainRenderingInfo({0.0f, 1.0f, 1.0f, 1.0f}, false)};
		auto cmd{m_renderCtx->getCurrentSwapchainCommandBuffer()};

		m_renderCtx->getDescriptorHeap()->bind();

		m_camera.onUpdate(p_dt);
		m_scene->onUpdate(p_dt);
		m_sceneRenderer->onRender(m_camera.getPosition(), m_camera.getViewMatrix(), m_camera.getProjectionMatrix());

		m_graphicsState->bind();
		m_renderCtx->beginRendering(rendering_info);

		FullscreenQuadConstants quad_constants{};
		quad_constants.samplerIndex = m_renderCtx->getSampler(render::ESamplerType::eDefault);
		quad_constants.textureIndex = m_sceneRenderer->getColourImage()->getAlignedShaderReadHeapID();
		cmd->pushData(quad_constants);

		m_renderCtx->renderFullscreenQuad();
		m_renderCtx->endRendering(rendering_info);
	}

	auto onResize(tsm::uint2 p_size) -> void override
	{
		m_viewportSize = p_size;
		m_camera.onResize(p_size);
		m_scene->onResize(p_size);
		m_sceneRenderer->onResize(p_size);
	}

	auto onEvent(Event &p_event) -> void override
	{
		m_camera.onEvent(p_event);
	}

private:
	tsm::uint2 m_viewportSize{0u};

	gpu::DynamicShaderHandle m_vertexShader;
	gpu::DynamicShaderHandle m_fragmentShader;

	UniquePtr<render::GraphicsState> m_graphicsState{nullptr};

	render::ImageHandle m_image{nullptr};

	FPCamera m_camera{};

	render::DynamicMeshHandle m_mesh{nullptr};

	UniquePtr<Scene>                m_scene{nullptr};
	UniquePtr<DynamicSceneRenderer> m_sceneRenderer{nullptr};

	TST_PUSH_CONSTANT_BLOCK(FullscreenQuadConstants)
	{
		uint32 samplerIndex;
		uint32 textureIndex;

		char _padd[8];
	};
};

auto main(int32 p_argc, char **p_argv) -> int32
{
	ApplicationSpecInfo app_spec{};
	app_spec.printGPUDebugInfo             = true;
	app_spec.windowSpecInfo.startMaximized = true;
	Application app{app_spec, nullptr};

	app.addLayer<ClientLayer>();

	app.run();
	return 0;
}
