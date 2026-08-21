#include <toast_gpu/descriptor_heap.hpp>

#include "toast_gpu/buffer.hpp"
#include "toast_gpu/texture.hpp"

using namespace toaster;

auto main(TST_UNUSED int32 p_argc, TST_UNUSED char **p_argv) -> int32
{
	#if 0
	gpu::initBaseFunctions();
	gpu::InstanceDesc instance_desc{};
	instance_desc.enableValidationLayers = true;
	instance_desc.applicationName        = "Test app";
	gpu::Instance gpu_instance{instance_desc};
	gpu::initInstanceFunctions(gpu_instance.getInstance());

	gpu::PhysicalDeviceDesc physical_device_desc{};
	physical_device_desc.requiredExtensions = {vk::EXTDescriptorHeapExtensionName, vk::KHRBufferDeviceAddressExtensionName, vk::KHRSynchronization2ExtensionName};
	gpu::PhysicalDevice physical_device{gpu_instance, physical_device_desc};

	gpu::LogicalDeviceDesc logical_device_desc{};
	logical_device_desc.enabledExtensions = physical_device_desc.requiredExtensions;

	vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceBufferDeviceAddressFeatures, vk::PhysicalDeviceDescriptorHeapFeaturesEXT,
		vk::PhysicalDeviceSynchronization2Features, vk::PhysicalDeviceTimelineSemaphoreFeatures> feature_chain{{}, {}, {}, {}, {}};
	feature_chain.get<vk::PhysicalDeviceBufferDeviceAddressFeatures>().bufferDeviceAddress = true;
	feature_chain.get<vk::PhysicalDeviceDescriptorHeapFeaturesEXT>().descriptorHeap        = true;
	feature_chain.get<vk::PhysicalDeviceDescriptorHeapFeaturesEXT>().descriptorHeap        = true;
	feature_chain.get<vk::PhysicalDeviceSynchronization2Features>().synchronization2       = true;
	feature_chain.get<vk::PhysicalDeviceTimelineSemaphoreFeatures>().timelineSemaphore     = true;
	logical_device_desc.pNextDeviceFeatures                                                = feature_chain.get<vk::PhysicalDeviceFeatures2>();

	gpu::LogicalDevice logical_device{physical_device, logical_device_desc};
	gpu::initDeviceFunctions(logical_device.getDevice());

	gpu::Allocator              allocator{gpu_instance, physical_device, logical_device};
	gpu::ResourceDescriptorHeap resource_descriptor_heap{logical_device, physical_device, allocator, 32u, 32u};

	gpu::CommandQueue cmd_queue{
		logical_device.getDevice(),
		logical_device.getGraphicsQueue(),
		logical_device.getQueueFamilyIndices().graphics,
		3u,
		3u,
		vk::CommandPoolCreateFlagBits::eResetCommandBuffer
	};

	{
		gpu::BufferManager buffer_manager{logical_device, allocator};

		gpu::BufferDesc buffer_desc{};
		buffer_desc.memoryProperties = gpu::EMemoryProperties::eDeviceLocal;
		buffer_desc.usageFlags       = vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst;
		buffer_desc.size             = sizeof(uint32);
		gpu::BufferHandle buffer{buffer_manager.createBuffer(buffer_desc)};

		gpu::BufferDesc staging_buffer_desc{};
		staging_buffer_desc.memoryProperties = gpu::EMemoryProperties::eHostVisibleCoherent;
		staging_buffer_desc.usageFlags       = vk::BufferUsageFlagBits::eTransferSrc;
		staging_buffer_desc.size             = sizeof(uint32);
		gpu::BufferHandle staging_buffer{buffer_manager.createBuffer(staging_buffer_desc)};

		uint32 data{0xFFFFFFFF};
		buffer_manager.uploadDirect(staging_buffer, &data, sizeof(uint32));

		auto cmd{cmd_queue.beginCurrentCommandBuffer()};

		vk::BufferCopy2 copy_region{};
		copy_region.size      = sizeof(uint32);
		copy_region.srcOffset = 0u;
		copy_region.dstOffset = 0u;
		vk::CopyBufferInfo2 copy_buffer{};
		copy_buffer.srcBuffer = buffer_manager.getBufferBuffer(staging_buffer);
		copy_buffer.dstBuffer = buffer_manager.getBufferBuffer(buffer);
		copy_buffer.setRegions(copy_region);
		cmd.copyBuffer2(copy_buffer);

		cmd_queue.deferDeletion([buffer_manager, staging_buffer]() mutable -> void { buffer_manager.destroyBuffer(staging_buffer); });

		cmd_queue.endCommandBuffer(cmd);
		vk::CommandBufferSubmitInfo cmd_info{cmd};
		vk::SubmitInfo2             submit_info{};
		submit_info.setCommandBufferInfos(cmd_info);
		cmd_queue.submit(submit_info);
		cmd_queue.waitForSubmit();

		buffer_manager.destroyBuffer(buffer);
	}
	#endif

	return 0;
}
