#pragma once
#include "vk_instance.hpp"

namespace toaster::gpu
{
	struct VKPhysicalDeviceSpecInfo
	{
		using ExtensionSet = std::unordered_set<CString>;

		ExtensionSet requiredExtensions;
	};

	class VKPhysicalDevice
	{
	public:
		VKPhysicalDevice(VKInstance *p_instance, const VKPhysicalDeviceSpecInfo &p_spec_info);

		auto getInstance() const -> VKInstance *;
		auto getSpecInfo() const -> const VKPhysicalDeviceSpecInfo &;

		[[nodiscard]] auto getVulkanPhysicalDevice() -> vk::raii::PhysicalDevice &;

		auto getMaxUsableSampleCount() const -> vk::SampleCountFlagBits;
		auto getDepthFormat() const -> vk::Format;

		[[nodiscard]] auto findSupportedFormat(const std::vector<vk::Format> &p_supported_formats, vk::ImageTiling p_tiling,
											   vk::FormatFeatureFlags         p_feature_flags) const -> vk::Format;

		operator vk::raii::PhysicalDevice &() { return m_physicalDevice; }

	private:
		[[nodiscard]] auto _isDeviceSuitable(const vk::raii::PhysicalDevice &p_physical_device) const -> bool;

		VKInstance *m_instance{nullptr};

		VKPhysicalDeviceSpecInfo m_specInfo;

		vk::raii::PhysicalDevice m_physicalDevice{nullptr};

		vk::SampleCountFlagBits m_maxUsableSampleCount{vk::SampleCountFlagBits::e1};
		vk::Format              m_depthFormat{vk::Format::eUndefined};
	};
}
