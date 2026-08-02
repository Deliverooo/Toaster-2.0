#pragma once

#include "instance.hpp"

namespace toaster::gpu
{
	using DeviceSuitabilityFn = bool(*)(vk::PhysicalDevice);
	// Checks the physical device against the baseline required suitability standards and optionally a custom function
	TST_GPU_API auto isDeviceSuitable(vk::PhysicalDevice  p_physical_device, const ExtensionSet &p_required_extensions,
									  DeviceSuitabilityFn p_extra_suitability_check = nullptr) -> bool;

	struct TST_GPU_API PhysicalDeviceSpecInfo
	{
		ExtensionSet requiredExtensions; // These are DEVICE extensions

		DeviceSuitabilityFn deviceSuitabilityCheckFn{nullptr}; // Optional, but I would recommend that you provide this
	};

	class TST_GPU_API PhysicalDevice
	{
	public:
		PhysicalDevice(Instance &p_instance, const PhysicalDeviceSpecInfo &p_spec_info);

		// You cannot modify an instance, so it is always const
		auto getInstance() const -> const Instance & { return *m_instance; }

		auto getVulkanPhysicalDevice() const -> const vk::raii::PhysicalDevice & { return m_physicalDevice; }
		auto operator *() const -> const vk::raii::PhysicalDevice & { return m_physicalDevice; }

		// Utility functions
		auto chooseSwapchainSurfaceFormat(vk::SurfaceKHR p_surface) const -> vk::SurfaceFormatKHR;
		auto chooseSwapchainPresentMode(vk::SurfaceKHR p_surface) const -> vk::PresentModeKHR;
		auto chooseSwapchainExtent(vk::SurfaceKHR p_surface, uint32 p_fallback_width, uint32 p_fallback_height) const -> vk::Extent2D;
		auto chooseSwapchainMinImageCount(vk::SurfaceKHR p_surface) const -> uint32;

	private:
		// A physical device should know what instance it is associated with
		Instance *m_instance{nullptr};

		vk::raii::PhysicalDevice m_physicalDevice{nullptr};
	};
}
