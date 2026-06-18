#include <toast_kernel/application.hpp>
#include <toast_kernel/fp_camera.hpp>
#include <toast_kernel/layer.hpp>

#include <toast_render/globals.hpp>
#include <toast_render/render_context.hpp>

#include <toast_lib/os/terminal.hpp>

using namespace toaster;

class ClientLayer : public IAppLayer
{
public:
	auto onInit() -> void override
	{
		m_viewportSize = m_app->getWindow().getRenderAreaSize();

		m_camera = FPCamera{m_inputCtx, 90.0f, m_viewportSize.aspect(), 0.1f, 1000.0f};

		auto binary_dir{os::getBinaryDirectory()};
		auto resources_dir{binary_dir / "../resources"};

		m_image = m_renderCtx->createImageRef(resources_dir / "textures/Peeber.png");

		m_graphicsState = m_renderCtx->createUnique<render::GraphicsState>();

		auto mesh_shader{m_globals->getShader("Test_Shader_MS")};
		auto pixel_shader{m_globals->getShader("Test_Shader_PS")};
		// auto mesh_shader{m_globals->getShader("Fullscreen_Quad_VS")};
		// auto pixel_shader{m_globals->getShader("Fullscreen_Quad_PS")};
		m_graphicsState->setShaders({mesh_shader, pixel_shader}).setVertexBufferLayout(render::RenderContext::fullscreenQuadVbl).setAttachmentCount(1u).
				setCullMode(vk::CullModeFlagBits::eNone).setEnableDepthTest(false).setEnableDepthWrite(false);
	}

	auto onDestroy() -> void override
	{
	}

	auto onUpdate(float32 p_dt) -> void override
	{
		const auto rendering_info{m_app->getWindow().getSwapchainRenderingInfo(false, {0.0f, 1.0f, 1.0f, 1.0f}, false)};
		const auto cmd{m_renderCtx->getCurrentSwapchainCommandBuffer()};
		auto &     vk_cmd{cmd->getVulkanCommandBuffer()};

		m_graphicsState->bind();
		m_renderCtx->beginRendering(rendering_info);

		vk_cmd.drawMeshTasksEXT(1, 1, 1);

		m_renderCtx->endRendering(rendering_info);
	}

	auto onResize(tsm::uint2 p_size) -> void override
	{
		m_viewportSize = p_size;
		m_camera.onResize(p_size);
	}

	auto onEvent(Event &p_event) -> void override
	{
		m_camera.onEvent(p_event);
	}

private:
	tsm::uint2 m_viewportSize{0u};

	UniquePtr<render::GraphicsState> m_graphicsState{nullptr};

	render::ImageHandle m_image{nullptr};

	FPCamera m_camera{};

	TST_PUSH_CONSTANT_BLOCK(FullscreenQuadConstants)
	{
		uint32 samplerIndex;
		uint32 textureIndex;

	private:
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
