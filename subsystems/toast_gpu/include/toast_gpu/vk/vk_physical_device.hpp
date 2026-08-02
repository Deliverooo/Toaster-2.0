#pragma once

#include "../toast_gpu.hpp"

#include <unordered_set>
#include <vulkan/vulkan_raii.hpp>

namespace toaster::gpu
{
	using DeviceSuitabilityFn = bool(*)(vk::PhysicalDevice);
	// Checks the physical device against the baseline required suitability standards and optionally a custom function
	TST_GPU_API auto isDeviceSuitable(vk::PhysicalDevice  p_physical_device, const std::unordered_set<String> &p_required_extensions,
									  DeviceSuitabilityFn p_extra_suitability_check = nullptr) -> bool;

	struct TST_GPU_API VKPhysicalDeviceSpecInfo
	{
		std::unordered_set<String> requiredExtensions;                // These are DEVICE extensions
		DeviceSuitabilityFn        deviceSuitabilityCheckFn{nullptr}; // Optional, but I would recomend that you provide this
	};

	class TST_GPU_API VKPhysicalDevice
	{
	public:
		VKPhysicalDevice(vk::raii::Instance& p_instance, const VKPhysicalDeviceSpecInfo &p_spec_info);

		[[nodiscard]] auto getVulkanPhysicalDevice() -> vk::raii::PhysicalDevice &;

		[[nodiscard]] auto getMaxUsableSampleCount() const -> vk::SampleCountFlagBits;
		[[nodiscard]] auto getDepthFormat() const -> vk::Format;

		[[nodiscard]] auto findSupportedFormat(const std::vector<vk::Format> &p_supported_formats, vk::ImageTiling p_tiling,
											   vk::FormatFeatureFlags         p_feature_flags) const -> vk::Format;

		[[nodiscard]] auto findMemoryType(uint32 p_type_filter, vk::MemoryPropertyFlags p_properties) const -> uint32;

		[[nodiscard]] auto chooseSwapchainSurfaceFormat(const vk::SurfaceKHR &p_surface) const -> vk::SurfaceFormatKHR;
		[[nodiscard]] auto chooseSwapchainPresentMode(const vk::SurfaceKHR &p_surface) const -> vk::PresentModeKHR;
		[[nodiscard]] auto chooseSwapchainExtent(const vk::SurfaceKHR &p_surface, uint32 p_fallback_width, uint32 p_fallback_height) const -> vk::Extent2D;
		[[nodiscard]] auto chooseSwapchainMinImageCount(const vk::SurfaceKHR &p_surface) const -> uint32;

		operator vk::raii::PhysicalDevice &() { return m_physicalDevice; }

	private:
		auto _init(vk::raii::Instance& p_instance, const VKPhysicalDeviceSpecInfo &p_spec_info) -> void;

		vk::raii::PhysicalDevice m_physicalDevice{nullptr};

		vk::SampleCountFlagBits m_maxUsableSampleCount{vk::SampleCountFlagBits::e1};
		vk::Format              m_depthFormat{vk::Format::eUndefined};
	};
}
