#include "toast_gpu/vk/vk_buffer.hpp"
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

		gpu::BufferSpecInfo ubo_spec_info{};
		ubo_spec_info.usageFlags = vk::BufferUsageFlagBits2::eUniformBuffer | vk::BufferUsageFlagBits2::eShaderDeviceAddressKHR;
		m_ubo                    = toaster::make_unique<gpu::Buffer>(logical_device, sizeof(tsm::float4), ubo_spec_info);
		tsm::float4 data{1.0f, 0.0f, 1.0f, 1.0f};
		m_ubo->setData(data);

		vk::PhysicalDeviceProperties2 device_props{};
		device_props.pNext = &m_heapProps;
		logical_device->getPhysicalDevice()->getVulkanPhysicalDevice().getProperties2(&device_props);

		vk::DeviceSize resource_heap_size{
			alignedSize(12 * m_heapProps.bufferDescriptorSize + m_heapProps.minResourceHeapReservedRange, m_heapProps.resourceHeapAlignment)
		};

		gpu::BufferSpecInfo resource_heap_spec_info{};
		resource_heap_spec_info.usageFlags = vk::BufferUsageFlagBits2::eDescriptorHeapEXT | vk::BufferUsageFlagBits2::eShaderDeviceAddressKHR;
		m_resourceHeap                     = toaster::make_unique<gpu::Buffer>(logical_device, resource_heap_size, resource_heap_spec_info);

		void *mapped_resource_heap_memory{m_resourceHeap->mapMemory(m_resourceHeap->getSize())};

		vk::DeviceAddressRangeEXT buffer_range{};
		buffer_range.address = logical_device->getVulkanLogicalDevice().getBufferAddressKHR({m_ubo->getBuffer()});
		buffer_range.size    = sizeof(tsm::float4);

		vk::DeviceSize          uniform_descriptor_size{alignedSize(m_heapProps.bufferDescriptorSize, m_heapProps.bufferDescriptorAlignment)};
		vk::HostAddressRangeEXT host_range{};
		host_range.address = static_cast<uint8 *>(mapped_resource_heap_memory) + (0 * m_heapProps.bufferDescriptorSize);
		host_range.size    = uniform_descriptor_size;

		vk::ResourceDescriptorInfoEXT resource_info{};
		resource_info.type               = vk::DescriptorType::eUniformBuffer;
		resource_info.data.pAddressRange = &buffer_range;

		logical_device->getVulkanLogicalDevice().writeResourceDescriptorsEXT(resource_info, host_range);

		m_resourceHeap->unmapMemory();

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
		auto logical_device{m_renderCtx->getLogicalDevice()};

		auto  rendering_info{m_app->getWindow().getSwapchainRenderingInfo({1.0f, 1.0f, 1.0f, 1.0f}, false)};
		auto &cmd{m_renderCtx->getCurrentSwapchainCommandBuffer()->getVulkanCommandBuffer()};

		m_renderCtx->beginRendering(rendering_info);

		vk::BindHeapInfoEXT resource_heap_bind_info{};
		resource_heap_bind_info.heapRange           = m_resourceHeap->getDeviceAddressRange();
		resource_heap_bind_info.reservedRangeOffset = m_resourceHeap->getSize() - m_heapProps.minResourceHeapReservedRange;
		resource_heap_bind_info.reservedRangeSize   = m_heapProps.minResourceHeapReservedRange;

		cmd.bindResourceHeapEXT(resource_heap_bind_info);

		vk::HostAddressRangeConstEXT ubo_address_range{};
		vk::DeviceAddress            ubo_address{m_ubo->getDeviceAddress()};
		ubo_address_range.address = &ubo_address;
		ubo_address_range.size    = sizeof(vk::DeviceAddress);

		vk::PushDataInfoEXT push_data_info{};
		push_data_info.offset = 0;
		push_data_info.data   = ubo_address_range;

		cmd.pushDataEXT(push_data_info);

		cmd.setViewportWithCountEXT({rendering_info.getViewport()});
		cmd.setScissorWithCountEXT({rendering_info.getScissor()});
		m_graphicsState->bind();

		m_globals->fullscreenQuadVertexBuffer()->bind();
		m_globals->fullscreenQuadIndexBuffer()->bind();

		cmd.drawIndexed(m_globals->fullscreenQuadIndices().size(), 1, 0, 0, 0);

		cmd.endRendering();
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

	// vk::raii::Buffer       m_resourceHeap{nullptr};
	// vk::DeviceSize         m_resourceHeapSize{0u};
	// vk::raii::DeviceMemory m_resourceHeapMemory{nullptr};

	gpu::BufferUnique                             m_resourceHeap{nullptr};
	vk::PhysicalDeviceDescriptorHeapPropertiesEXT m_heapProps{};

	gpu::BufferUnique m_ubo{nullptr};
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
