#include <toast_gpu/descriptor_heap.hpp>

#include "toast_gpu/buffer.hpp"

using namespace toaster;

auto main(TST_UNUSED int32 p_argc, TST_UNUSED char **p_argv) -> int32
{
	gpu::initBaseFunctions();
	gpu::InstanceDesc instance_desc{};
	instance_desc.enableValidationLayers = true;
	instance_desc.applicationName        = "Test app";
	gpu::Instance gpu_instance{instance_desc};
	gpu::initInstanceFunctions(gpu_instance.getInstance());

	gpu::PhysicalDeviceDesc physical_device_desc{};
	physical_device_desc.requiredExtensions = {vk::EXTDescriptorHeapExtensionName, vk::KHRBufferDeviceAddressExtensionName};
	gpu::PhysicalDevice physical_device{gpu_instance, physical_device_desc};

	gpu::LogicalDeviceDesc logical_device_desc{};
	logical_device_desc.enabledExtensions = physical_device_desc.requiredExtensions;

	vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceBufferDeviceAddressFeatures, vk::PhysicalDeviceDescriptorHeapFeaturesEXT> feature_chain{{}, {}, {}};
	feature_chain.get<vk::PhysicalDeviceBufferDeviceAddressFeatures>().bufferDeviceAddress = true;
	feature_chain.get<vk::PhysicalDeviceDescriptorHeapFeaturesEXT>().descriptorHeap        = true;
	logical_device_desc.pNextDeviceFeatures                                                = feature_chain.get<vk::PhysicalDeviceFeatures2>();

	gpu::LogicalDevice logical_device{physical_device, logical_device_desc};
	gpu::initDeviceFunctions(logical_device.getDevice());

	gpu::Allocator              allocator{gpu_instance, physical_device, logical_device};
	gpu::ResourceDescriptorHeap resource_descriptor_heap{logical_device, physical_device, allocator, 32u, 32u};

	#if 0
	gpu::BufferManager buffer_manager{logical_device, allocator, resource_descriptor_heap}; gpu::BufferDesc buffer_desc{}; buffer_desc.size = 1028u;
	buffer_desc.usageFlags = vk::BufferUsageFlagBits::eTransferDst; buffer_desc.memoryProperties = gpu::EMemoryProperties::eDeviceLocal; gpu::BufferHandle dst_buffer
			{buffer_manager.createBuffer(buffer_desc)}; auto data{new uint8[1028u]}; vk::CommandBufferAllocateInfo cmd_alloc_info{};
	cmd_alloc_info.commandBufferCount = 1u; cmd_alloc_info.commandPool = logical_device.getGraphicsCommandPool(); cmd_alloc_info.level = vk::CommandBufferLevel::ePrimary;
	vk::CommandBuffer cmd{logical_device.getDevice().allocateCommandBuffers(cmd_alloc_info).front()}; vk::Fence wait_fence{logical_device.getDevice().createFence({})};
	cmd.begin(vk::CommandBufferBeginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit}); buffer_manager.setBufferData(dst_buffer, cmd, wait_fence, data, 1028u); cmd.
			end(); vk::CommandBufferSubmitInfo command_buffer_info{}; command_buffer_info.setCommandBuffer(cmd); vk::SubmitInfo2 submit_info{}; submit_info.
			setCommandBufferInfos(command_buffer_info); logical_device.getGraphicsQueue().submit2(submit_info, wait_fence); if (
		logical_device.getDevice().waitForFences(wait_fence, true, INFINITE) != vk::Result::eSuccess)
		TST_PERMA_ASSERT(false); buffer_manager.processDeferredDestructions(); logical_device.getDevice().destroyFence(wait_fence); buffer_manager.
			destroyBuffer(dst_buffer); delete[] data;
	#endif
	return 0;
}
