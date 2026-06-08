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

#include "toast_math/colours.hpp"

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
		m_ubo->setAllData(tsm::float4{0.0f, 1.0f, 1.0f, 1.0f});

		m_ubo2 = m_renderCtx->createRef<render::UniformBufferPFF>(sizeof(tsm::float4));
		m_ubo2->setAllData(tsm::float4{0.0f, 0.0f, 1.0f, 1.0f});

		render::ImageSpecInfo image_spec{};
		image_spec.format = vk::Format::eR8G8B8A8Unorm;
		image_spec.size   = {2u};

		Buffer image_data;
		image_data.allocate(sizeof(uint32) * 4);
		image_data.write<uint32>(tsm::colours::rgbaToHex(tsm::colours::weezer), 0);
		image_data.write<uint32>(tsm::colours::rgbaToHex(tsm::colours::magenta), sizeof(uint32));
		image_data.write<uint32>(tsm::colours::rgbaToHex(tsm::colours::blue), sizeof(uint32) * 2);
		image_data.write<uint32>(tsm::colours::rgbaToHex(tsm::colours::red), sizeof(uint32) * 3);
		m_image = m_renderCtx->createRef<render::Image>(image_spec, image_data);

		image_data.release();
		// m_image           = m_renderCtx->createImageRef(binary_dir / "../resources/textures/Peeber.png");

		gpu::ShaderCompiler shader_compiler{logical_device};
		auto vs_bytecode{shader_compiler.compileToBytecodeFromFilepath(vk::ShaderStageFlagBits::eVertex, binary_dir / "../resources/shaders/dynamic.vert.glsl")};
		auto fs_bytecode{shader_compiler.compileToBytecodeFromFilepath(vk::ShaderStageFlagBits::eFragment, binary_dir / "../resources/shaders/dynamic.pixel.glsl")};

		m_vertexShader   = m_renderCtx->createGPURef<gpu::DynamicShader>(vs_bytecode, vk::ShaderStageFlagBits::eVertex, vk::ShaderStageFlagBits::eFragment);
		m_fragmentShader = m_renderCtx->createGPURef<gpu::DynamicShader>(fs_bytecode, vk::ShaderStageFlagBits::eFragment);

		m_graphicsState = m_renderCtx->createUnique<render::GraphicsState>();
		m_graphicsState->setShaders({m_vertexShader, m_fragmentShader}).setVertexBufferLayout(render::RenderContext::fullscreenQuadVbl).setAttachmentCount(1u);

	}

	auto onUpdate(float32 p_dt) -> void override
	{
		auto rendering_info{m_app->getWindow().getSwapchainRenderingInfo({1.0f, 1.0f, 1.0f, 1.0f}, false)};

		auto  cmd{m_renderCtx->getCurrentSwapchainCommandBuffer()};
		auto &vk_cmd{cmd->getVulkanCommandBuffer()};

		m_renderCtx->beginRendering(rendering_info);
		cmd->setRenderArea(rendering_info.renderArea);

		m_renderCtx->getDescriptorHeap()->bind();

		struct PushConstants
		{
			uint32 textureIndex;
			uint32 samplerIndex;

			uint32 bufferIndex;
		};

		uint32 buffer_index;
		uint32 texture_index;

		if (m_inputCtx->isKeyDown(input::EKeyCode::eY))
		{
			buffer_index  = m_ubo->getAlignedHeapID();
			texture_index = m_image->getAlignedHeapID();
		}
		else
		{
			buffer_index  = m_ubo2->getAlignedHeapID();
			texture_index = m_globals->whiteImage()->getAlignedHeapID();
		}

		if (m_inputCtx->isKeyPressed(input::EKeyCode::eT))
		{
			if (m_activeSampler == render::ESamplerType::eDefault)
				m_activeSampler = render::ESamplerType::eNearest;
			else
				m_activeSampler = render::ESamplerType::eDefault;
		}

		uintptr sampler_address{m_renderCtx->getSampler(m_activeSampler)};

		PushConstants pcs{};
		pcs.textureIndex = texture_index;
		pcs.samplerIndex = sampler_address;
		pcs.bufferIndex  = buffer_index;
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
	render::UniformBufferPFFHandle m_ubo2{nullptr};

	gpu::RawImageHandle m_rawImage{nullptr};
	uintptr             m_rawImageHeapOffset{0u};

	render::ESamplerType m_activeSampler{render::ESamplerType::eDefault};
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
