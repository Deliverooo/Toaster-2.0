#include "toast_gpu/physical_device.hpp"

namespace toaster::gpu
{
	auto isDeviceSuitable(vk::PhysicalDevice  p_physical_device, const std::unordered_set<CString> &p_required_extensions,
						  DeviceSuitabilityFn p_extra_suitability_check) -> bool
	{
		const auto props{p_physical_device.getProperties()};
		const bool vulkan_1_4_support{props.apiVersion >= vk::ApiVersion14};

		auto       queue_family_props{p_physical_device.getQueueFamilyProperties()};
		const bool supports_graphics{
			std::ranges::any_of(queue_family_props, [](const auto &queue_family)
			{
				return !!(queue_family.queueFlags & vk::QueueFlagBits::eGraphics);
			})
		};

		const bool supports_compute{
			std::ranges::any_of(queue_family_props, [](const auto &queue_family)
			{
				return !!(queue_family.queueFlags & vk::QueueFlagBits::eCompute);
			})
		};

		// Checks if all the required extensions are present in the available_device_extensions vector.
		auto       available_device_extensions{p_physical_device.enumerateDeviceExtensionProperties()};
		const bool supports_all_required_device_extensions{
			std::ranges::all_of(p_required_extensions, [available_device_extensions](const auto &required_ext)
			{
				return std::ranges::any_of(available_device_extensions, [&required_ext](const auto &available_ext)
				{
					return std::strcmp(available_ext.extensionName, required_ext) == 0;
				});
			})
		};

		const bool passes_additional_check{p_extra_suitability_check ? p_extra_suitability_check(p_physical_device) : true};

		return vulkan_1_4_support && supports_graphics && supports_compute && supports_all_required_device_extensions && passes_additional_check;
	}

	PhysicalDevice::PhysicalDevice(Instance &p_instance, const PhysicalDeviceDesc &p_desc)
	{
		auto physical_devices{p_instance.getInstance().enumeratePhysicalDevices()};
		if (physical_devices.empty())
		{
			// If your gpu does not have Vulkan support, we can't use Vulkan
			TST_PERMA_ASSERT_MSG(false, "Failed to find physical devices with Vulkan support");
		}

		// I don't expect users to provide ts when they create a physical device, however depending on what you are doing, you will have different requirements.
		DeviceSuitabilityFn device_suitability_check_fn{nullptr};
		if (!p_desc.deviceSuitabilityCheckFn)
		{
			device_suitability_check_fn = +[](vk::PhysicalDevice p_physical_device) -> bool
			{
				auto features{
					p_physical_device.getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan12Features, vk::PhysicalDeviceVulkan13Features,
						vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT, vk::PhysicalDeviceCustomBorderColorFeaturesEXT>()
				};

				const bool supports_required_features{
					features.get<vk::PhysicalDeviceFeatures2>().features.samplerAnisotropy && features.get<vk::PhysicalDeviceFeatures2>().features.sampleRateShading &&
					features.get<vk::PhysicalDeviceFeatures2>().features.fillModeNonSolid && features.get<vk::PhysicalDeviceVulkan12Features>().timelineSemaphore &&
					features.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering && features.get<vk::PhysicalDeviceVulkan13Features>().synchronization2 && features
					.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState && features.get<vk::PhysicalDeviceCustomBorderColorFeaturesEXT>().
					customBorderColors
				};

				return supports_required_features;
			};
		}
		else
			device_suitability_check_fn = p_desc.deviceSuitabilityCheckFn;

		const auto device_it{
			std::ranges::find_if(physical_devices, [&](const auto &device)
			{
				return isDeviceSuitable(device, p_desc.requiredExtensions, device_suitability_check_fn);
			})
		};

		if (device_it == physical_devices.end())
		{
			TST_PERMA_ASSERT_MSG(false, "Failed to find suitable physical device");
		}

		m_physicalDevice = *device_it;

		m_deviceProperties.pNext = &m_descriptorHeapProperties;
		m_physicalDevice.getProperties2(&m_deviceProperties);
	}

	auto PhysicalDevice::chooseSwapchainSurfaceFormat(vk::SurfaceKHR p_surface) const -> vk::SurfaceFormatKHR
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

	auto PhysicalDevice::chooseSwapchainPresentMode(vk::SurfaceKHR p_surface) const -> vk::PresentModeKHR
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

	auto PhysicalDevice::chooseSwapchainExtent(vk::SurfaceKHR p_surface, uint32 p_fallback_width, uint32 p_fallback_height) const -> vk::Extent2D
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

	auto PhysicalDevice::chooseSwapchainMinImageCount(vk::SurfaceKHR p_surface) const -> uint32
	{
		const auto surface_caps{m_physicalDevice.getSurfaceCapabilitiesKHR(p_surface)};
		// Ideally, we want the min image count to be at least 3. However, if your GPU is bad, it might not be able to handle that many images.
		// So if 3 is greater than the max image count, we fall back to the max image count as the min image count... I don't know if that made sense...
		uint32 min_image_count{std::max(3u, surface_caps.minImageCount)};

		// Apparently, if the maxImageCount == 0, then there is no maximum (unlimited).
		if ((surface_caps.maxImageCount > 0) && (surface_caps.maxImageCount < min_image_count))
			min_image_count = surface_caps.maxImageCount;
		return min_image_count;
	}
}
