#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1

#include <toast_gpu/logical_device.hpp>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

auto main(TST_UNUSED int32 p_argc, TST_UNUSED char **p_argv) -> int32
{
	VULKAN_HPP_DEFAULT_DISPATCHER.init();
	toaster::gpu::InstanceSpecInfo instance_desc{};
	instance_desc.enableValidationLayers = true;
	toaster::gpu::Instance gpu_instance{instance_desc};
	VULKAN_HPP_DEFAULT_DISPATCHER.init(gpu_instance.getVulkanInstance());

	toaster::gpu::PhysicalDeviceSpecInfo physical_device_desc{};
	// physical_device_desc.requiredExtensions = toaster::gpu::PhysicalDeviceSpecInfo::
	toaster::gpu::PhysicalDevice physical_device{gpu_instance, physical_device_desc};

	toaster::gpu::LogicalDeviceSpecInfo logical_device_desc{};
	toaster::gpu::LogicalDevice         logical_device{physical_device, logical_device_desc};
	VULKAN_HPP_DEFAULT_DISPATCHER.init(logical_device.getVulkanLogicalDevice());

	return 0;
}
