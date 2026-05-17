#include "vk_physical_device.hpp"

namespace toaster::gpu
{
	VKPhysicalDevice::VKPhysicalDevice(VKInstance *p_instance, const VKPhysicalDeviceSpecInfo &p_spec_info) : m_instance(p_instance), m_specInfo(p_spec_info)
	{
		auto physical_devices = m_instance->getVulkanInstance().enumeratePhysicalDevices();
		if (physical_devices.empty())
		{
			// If your gpu does not have Vulkan support, we can't use Vulkan
			LOG_ERROR("Failed to find physical devices with Vulkan support");
			// system("pause");
			TST_ASSERT(false);
		}

		const auto device_it = std::ranges::find_if(physical_devices, [this](const auto &device)
		{
			return _isDeviceSuitable(device);
		});
		if (device_it == physical_devices.end())
		{
			LOG_ERROR("Failed to find suitable physical device");
			// system("pause");
			TST_ASSERT(false);
		}

		m_physicalDevice = *device_it;

		vk::PhysicalDeviceProperties props{m_physicalDevice.getProperties()};

		vk::SampleCountFlags sample_counts{props.limits.framebufferColorSampleCounts & props.limits.framebufferDepthSampleCounts};
		if (sample_counts & vk::SampleCountFlagBits::e64)
			m_maxUsableSampleCount = vk::SampleCountFlagBits::e64;
		else if (sample_counts & vk::SampleCountFlagBits::e32)
			m_maxUsableSampleCount = vk::SampleCountFlagBits::e32;
		else if (sample_counts & vk::SampleCountFlagBits::e16)
			m_maxUsableSampleCount = vk::SampleCountFlagBits::e16;
		else if (sample_counts & vk::SampleCountFlagBits::e8)
			m_maxUsableSampleCount = vk::SampleCountFlagBits::e8;
		else if (sample_counts & vk::SampleCountFlagBits::e4)
			m_maxUsableSampleCount = vk::SampleCountFlagBits::e4;
		else if (sample_counts & vk::SampleCountFlagBits::e2)
			m_maxUsableSampleCount = vk::SampleCountFlagBits::e2;
		else
			m_maxUsableSampleCount = vk::SampleCountFlagBits::e1;

		m_depthFormat = findSupportedFormat({vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint}, vk::ImageTiling::eOptimal,
											vk::FormatFeatureFlagBits::eDepthStencilAttachment);

		if (m_specInfo.printDebugInfo)
		{
			LOG_INFO("Using physical device: {} | Device ID: {}\n", props.deviceName.data(), props.deviceID);
			LOG_INFO("Available device extensions:");
			auto extension_props = m_physicalDevice.enumerateDeviceExtensionProperties();
			for (auto ext: extension_props)
				LOG_INFO("\t{}", ext.extensionName.data());
			LOG_INFO("");
		}
	}

	auto VKPhysicalDevice::getInstance() const -> NonOwningPtr<VKInstance>
	{
		return m_instance;
	}

	auto VKPhysicalDevice::getSpecInfo() const -> const VKPhysicalDeviceSpecInfo &
	{
		return m_specInfo;
	}

	auto VKPhysicalDevice::getVulkanPhysicalDevice() -> vk::raii::PhysicalDevice &
	{
		return m_physicalDevice;
	}

	auto VKPhysicalDevice::getMaxUsableSampleCount() const -> vk::SampleCountFlagBits
	{
		return m_maxUsableSampleCount;
	}

	auto VKPhysicalDevice::getDepthFormat() const -> vk::Format
	{
		return m_depthFormat;
	}

	auto VKPhysicalDevice::findSupportedFormat(const std::vector<vk::Format> &p_supported_formats, vk::ImageTiling p_tiling,
											   vk::FormatFeatureFlags         p_feature_flags) const -> vk::Format
	{
		for (const auto &format: p_supported_formats)
		{
			vk::FormatProperties props = m_physicalDevice.getFormatProperties(format);

			if (p_tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & p_feature_flags) == p_feature_flags)
				return format;
			if (p_tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & p_feature_flags) == p_feature_flags)
				return format;
		}
		TST_ASSERT_MSG(false, "Unsupported format");
		return vk::Format::eUndefined;
	}

	auto VKPhysicalDevice::findMemoryType(uint32 p_type_filter, vk::MemoryPropertyFlags p_properties) const -> uint32
	{
		const vk::PhysicalDeviceMemoryProperties memory_properties = m_physicalDevice.getMemoryProperties();
		for (uint32 i{0u}; i < memory_properties.memoryTypeCount; i++)
		{
			if ((p_type_filter & BIT(i)) && (memory_properties.memoryTypes[i].propertyFlags & p_properties) == p_properties)
			{
				return i;
			}
		}
		TST_ASSERT_MSG(false, "Failed to find a matching memory type!");
		return UINT32_MAX;
	}

	auto VKPhysicalDevice::chooseSwapchainSurfaceFormat(const vk::SurfaceKHR &p_surface) const -> vk::SurfaceFormatKHR
	{
		const auto available_formats{m_physicalDevice.getSurfaceFormatsKHR(p_surface)};
		TST_ASSERT(!available_formats.empty());
		// According to my expert research, the most aesthetically pleasing image format is RGBA in the SRGB colour space.
		// If for some reason, your GPU does not support that, then just fall back to the first available format.
		const auto format_it = std::ranges::find_if(available_formats, [](const auto &format)
		{
			return format.format == vk::Format::eR8G8B8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
		});
		return format_it == available_formats.end() ? available_formats[0] : *format_it;
	}

	auto VKPhysicalDevice::chooseSwapchainPresentMode(const vk::SurfaceKHR &p_surface) const -> vk::PresentModeKHR
	{
		auto available_present_modes{m_physicalDevice.getSurfacePresentModesKHR(p_surface)};
		// The ideal present mode would be mailbox because it is the fastest.
		// However, not every device supports it. But Fifo is guaranteed to be supported, so that is the fallback option
		TST_ASSERT(!available_present_modes.empty());
		return std::ranges::any_of(available_present_modes, [](const auto &present_mode)
		{
			return present_mode == vk::PresentModeKHR::eMailbox;
		})
				   ? vk::PresentModeKHR::eMailbox
				   : vk::PresentModeKHR::eFifo;
	}

	auto VKPhysicalDevice::chooseSwapchainExtent(const vk::SurfaceKHR &p_surface, uint32 p_fallback_width, uint32 p_fallback_height) const -> vk::Extent2D
	{
		const auto surface_caps{m_physicalDevice.getSurfaceCapabilitiesKHR(p_surface)};

		// If the current extent is UINT32_MAX, it means that we can choose our own custom extent
		if (surface_caps.currentExtent.width != UINT32_MAX)
			return surface_caps.currentExtent;

		// But we still have to make sure that we clamp the extent between the min and max.
		// I think this probably has to do with certain displays (Apple Retina) having a very high pixel density (DPI).
		return {
			std::clamp<uint32>(p_fallback_width, surface_caps.minImageExtent.width, surface_caps.maxImageExtent.width),
			std::clamp<uint32>(p_fallback_height, surface_caps.minImageExtent.height, surface_caps.maxImageExtent.height)
		};
	}

	auto VKPhysicalDevice::chooseSwapchainMinImageCount(const vk::SurfaceKHR &p_surface) const -> uint32
	{
		const auto surface_caps{m_physicalDevice.getSurfaceCapabilitiesKHR(p_surface)};
		// Ideally, we want the min image count to be at least 3. However, if your GPU is bad, it might not be able to handle that many images.
		// So if 3 is greater than the max image count, we fall back to the max image count as the min image count... I don't know if that made sense...
		uint32 min_image_count = std::max(3u, surface_caps.minImageCount);

		// Apparently, if the maxImageCount == 0, then there is no maximum (unlimited).
		if ((surface_caps.maxImageCount > 0) && (surface_caps.maxImageCount < min_image_count))
			min_image_count = surface_caps.maxImageCount;
		return min_image_count;
	}

	auto VKPhysicalDevice::_isDeviceSuitable(const vk::raii::PhysicalDevice &p_physical_device) const -> bool
	{
		auto props              = p_physical_device.getProperties();
		bool vulkan_1_3_support = props.apiVersion >= vk::ApiVersion13;

		auto queue_families    = p_physical_device.getQueueFamilyProperties();
		bool supports_graphics = std::ranges::any_of(queue_families, [](const auto &queue_family)
		{
			return !!(queue_family.queueFlags & vk::QueueFlagBits::eGraphics);
		});

		bool supports_compute = std::ranges::any_of(queue_families, [](const auto &queue_family)
		{
			return !!(queue_family.queueFlags & vk::QueueFlagBits::eCompute);
		});

		// Checks if all the required extensions are present in the available_device_extensions vector.
		auto available_device_extensions             = p_physical_device.enumerateDeviceExtensionProperties();
		bool supports_all_required_device_extensions = std::ranges::all_of(m_specInfo.requiredExtensions, [available_device_extensions](const auto &required_ext)
		{
			return std::ranges::any_of(available_device_extensions, [&required_ext](const auto &available_ext)
			{
				return std::strcmp(available_ext.extensionName, required_ext.c_str()) == 0;
			});
		});

		if (m_specInfo.printDebugInfo)
		{
			for (const auto &ext: available_device_extensions)
			{
				LOG_TRACE("{}", ext.extensionName.data());
			}
		}

		auto features = p_physical_device.getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan12Features, vk::PhysicalDeviceVulkan13Features,
			vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT, vk::PhysicalDeviceCustomBorderColorFeaturesEXT>();

		bool supports_required_features = features.get<vk::PhysicalDeviceFeatures2>().features.samplerAnisotropy && features.get<vk::PhysicalDeviceFeatures2>().features.
										  sampleRateShading && features.get<vk::PhysicalDeviceFeatures2>().features.fillModeNonSolid && features.get<
											  vk::PhysicalDeviceVulkan12Features>().timelineSemaphore && features.get<vk::PhysicalDeviceVulkan13Features>().
										  dynamicRendering && features.get<vk::PhysicalDeviceVulkan13Features>().synchronization2 && features.get<
											  vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState && features.get<
											  vk::PhysicalDeviceCustomBorderColorFeaturesEXT>().customBorderColors;

		return vulkan_1_3_support && supports_graphics && supports_compute && supports_all_required_device_extensions && supports_required_features;
	}
}
