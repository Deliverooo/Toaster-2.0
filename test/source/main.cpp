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
#include "toast_scene/scene_renderer.hpp"

using namespace toaster;

class ClientLayer : public IAppLayer
{
public:
	auto onInit() -> void override
	{
		m_viewportSize = m_app->getWindow().getRenderAreaSize();

		auto binary_dir{os::getBinaryDirectory()};
		auto logical_device{m_renderCtx->getLogicalDevice()};

		m_ubo = m_renderCtx->createRef<render::UniformBufferPFF>(sizeof(tsm::float4));
		m_ubo->setAllData(tsm::float4{1.0f, 0.0f, 1.0f, 1.0f});

		m_image = m_renderCtx->createImageRef(binary_dir / "../resources/textures/Peeber.png");

		gpu::ShaderCompiler shader_compiler{logical_device};
		auto vs_bytecode{shader_compiler.compileToBytecodeFromFilepath(vk::ShaderStageFlagBits::eVertex, binary_dir / "../resources/shaders/dynamic.vert.glsl")};
		auto fs_bytecode{shader_compiler.compileToBytecodeFromFilepath(vk::ShaderStageFlagBits::eFragment, binary_dir / "../resources/shaders/dynamic.pixel.glsl")};

		m_vertexShader   = m_renderCtx->createGPURef<gpu::DynamicShader>(vs_bytecode, vk::ShaderStageFlagBits::eVertex, vk::ShaderStageFlagBits::eFragment);
		m_fragmentShader = m_renderCtx->createGPURef<gpu::DynamicShader>(fs_bytecode, vk::ShaderStageFlagBits::eFragment);

		m_graphicsState = m_renderCtx->createUnique<render::GraphicsState>();
		m_graphicsState->setShaders({m_vertexShader, m_fragmentShader}).setVertexBufferLayout(render::RenderContext::fullscreenQuadVbl).setAttachmentCount(1u);

		render::Renderer2DSpecInfo r2dsi{};
		r2dsi.renderTargetSize = m_viewportSize;
		m_renderer2D           = m_renderCtx->createRef<render::Renderer2DV2>(r2dsi);
	}

	auto onUpdate(float32 p_dt) -> void override
	{
		auto rendering_info{m_app->getWindow().getSwapchainRenderingInfo({1.0f, 1.0f, 1.0f, 1.0f}, false)};

		auto  cmd{m_renderCtx->getCurrentSwapchainCommandBuffer()};
		auto &vk_cmd{cmd->getVulkanCommandBuffer()};

		m_renderer2D->begin(Dx::XMMatrixIdentity(), Dx::XMMatrixIdentity());
		m_renderer2D->submitQuad(Dx::XMMatrixIdentity(), m_image, {0.0f, 1.0f, 0.0f, 1.0f});
		m_renderer2D->end();

		m_renderCtx->beginRendering(rendering_info);
		cmd->setRenderArea(rendering_info.renderArea);

		m_renderCtx->getDescriptorHeap()->bind();

		struct PushConstants
		{
			uint32 textureIndex;
			uint32 samplerIndex;

			uintptr currentUBOPtr;
		};

		PushConstants pcs{};
		// pcs.textureIndex  = m_globals->whiteImage()->getAlignedHeapID();
		pcs.textureIndex  = m_renderer2D->getOutputColourImage()->getAlignedHeapID();
		pcs.samplerIndex  = m_renderCtx->getSampler(render::ESamplerType::eDefault);
		pcs.currentUBOPtr = m_ubo->getDeviceAddress();
		cmd->pushData(pcs);

		m_graphicsState->bind();

		m_globals->fullscreenQuadVertexBuffer()->bind();
		m_globals->fullscreenQuadIndexBuffer()->bind();

		cmd->drawIndexed(m_globals->fullscreenQuadIndices().size());

		vk_cmd.endRendering();
	}

	auto onResize(tsm::uint2 p_size) -> void override
	{
		m_viewportSize = p_size;
	}

private:
	tsm::uint2 m_viewportSize{0u};

	gpu::DynamicShaderHandle m_vertexShader;
	gpu::DynamicShaderHandle m_fragmentShader;

	UniquePtr<render::GraphicsState> m_graphicsState{nullptr};

	render::ImageHandle            m_image{nullptr};
	render::UniformBufferPFFHandle m_ubo{nullptr};

	RefPtr<render::Renderer2DV2> m_renderer2D{nullptr};
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
