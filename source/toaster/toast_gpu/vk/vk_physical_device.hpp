#pragma once
#include "vk_instance.hpp"

namespace toaster::gpu
{
	struct VKPhysicalDeviceSpecInfo
	{
		using ExtensionSet = std::unordered_set<String>;

		ExtensionSet requiredExtensions;
	};

	class TST_GPU_API VKPhysicalDevice
	{
	public:
		VKPhysicalDevice(VKInstance *p_instance, const VKPhysicalDeviceSpecInfo &p_spec_info);

		[[nodiscard]] auto getInstance() const -> NonOwningPtr<VKInstance>;
		[[nodiscard]] auto getSpecInfo() const -> const VKPhysicalDeviceSpecInfo &;

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
		[[nodiscard]] auto _isDeviceSuitable(const vk::raii::PhysicalDevice &p_physical_device) const -> bool;

		NonOwningPtr<VKInstance> m_instance{nullptr};

		VKPhysicalDeviceSpecInfo m_specInfo;

		vk::raii::PhysicalDevice m_physicalDevice{nullptr};

		vk::SampleCountFlagBits m_maxUsableSampleCount{vk::SampleCountFlagBits::e1};
		vk::Format              m_depthFormat{vk::Format::eUndefined};
	};
}
