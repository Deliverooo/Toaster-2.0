#include "toast_gpu/vk/vk_buffer.hpp"
#include "toast_gpu/vk/vk_descriptor_heap.hpp"
#include "toast_gpu/vk/vk_shader_compiler.hpp"
#include "toast_kernel/application.hpp"
#include "toast_kernel/input.hpp"
#include "toast_lib/os/file_dialog.hpp"
#include "toast_lib/os/terminal.hpp"
#include "toast_render/globals.hpp"
#include "toast_render/graphics_state.hpp"
#include "toast_render/render_context.hpp"
#include "toast_scene/entity.hpp"
#include "toast_scene/scene.hpp"
#include "toast_scene/scene_renderer.hpp"

using namespace toaster;

size_t alignedSize(size_t value, size_t alignment)
{
	return (value + alignment - 1) & ~(alignment - 1);
}

class ClientLayer : public IAppLayer
{
public:
	auto onInit() -> void override
	{
		m_viewportSize = m_app->getWindow().getRenderAreaSize();

		auto binary_dir{os::getBinaryDirectory()};
		auto logical_device{m_renderCtx->getLogicalDevice()};

		{
			gpu::BufferSpecInfo ubo_spec_info{};
			ubo_spec_info.usageFlags = vk::BufferUsageFlagBits2::eUniformBuffer | vk::BufferUsageFlagBits2::eShaderDeviceAddressKHR;
			m_ubo                    = toaster::make_unique<gpu::Buffer>(logical_device, sizeof(tsm::float4), ubo_spec_info);
			tsm::float4 data{1.0f, 0.0f, 1.0f, 1.0f};
			m_ubo->setData(data);
		}
		{
			gpu::BufferSpecInfo ubo_spec_info{};
			ubo_spec_info.usageFlags = vk::BufferUsageFlagBits2::eUniformBuffer | vk::BufferUsageFlagBits2::eShaderDeviceAddressKHR;
			m_ubo2                    = toaster::make_unique<gpu::Buffer>(logical_device, sizeof(tsm::float4), ubo_spec_info);
			tsm::float4 data{0.0f, 0.0f, 1.0f, 1.0f};
			m_ubo2->setData(data);
		}

		m_descriptorHeap = m_renderCtx->createGPUUnique<gpu::DescriptorHeap>();
		m_descriptorHeap->allocBuffer(*m_ubo);
		m_descriptorHeap->allocBuffer(*m_ubo2);

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

		m_descriptorHeap->bind();

		struct PushConstants
		{
			vk::DeviceAddress uboAddress;
			vk::DeviceAddress ubo2Address;
		};
		PushConstants pcs{};
		pcs.uboAddress = m_ubo->getDeviceAddress();
		pcs.ubo2Address = m_ubo2->getDeviceAddress();
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

	gpu::DescriptorHeapUnique m_descriptorHeap{nullptr};

	// gpu::BufferUnique                             m_resourceHeap{nullptr};
	// vk::PhysicalDeviceDescriptorHeapPropertiesEXT m_heapProps{};

	gpu::BufferUnique m_ubo{nullptr};
	gpu::BufferUnique m_ubo2{nullptr};
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
