#include <toast_gpu/descriptor_heap.hpp>

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

	gpu::Allocator allocator{gpu_instance, physical_device, logical_device};

	gpu::ResourceDescriptorHeap resource_descriptor_heap{logical_device, physical_device, allocator, 32u, 32u};

	return 0;
}
