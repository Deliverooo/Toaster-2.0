#include "toast_gpu/logical_device.hpp"

namespace toaster::gpu
{
	auto selectQueueFamilyIndices(vk::PhysicalDevice p_physical_device, bool p_use_present) -> QueueFamilyIndices
	{
		const auto queue_family_props{p_physical_device.getQueueFamilyProperties()};

		QueueFamilyIndices queue_family_indices{};

		for (uint32 i{0u}; i < queue_family_props.size(); ++i)
		{
			const auto queue_flags{queue_family_props[i].queueFlags};
			if (queue_flags & vk::QueueFlagBits::eGraphics && queue_flags & vk::QueueFlagBits::eCompute && (p_use_present
																												? p_physical_device.getWin32PresentationSupportKHR(i)
																												: true))
			{
				queue_family_indices.graphics = i;
				queue_family_indices.present  = i;
				queue_family_indices.compute  = i;

				break;
			}
		}

		return queue_family_indices;
	}

	LogicalDevice::LogicalDevice(PhysicalDevice &p_physical_device, const LogicalDeviceSpecInfo &p_spec_info) : m_physicalDevice(&p_physical_device)
	{
		const bool use_present{p_spec_info.enabledExtensions.contains(vk::KHRSwapchainExtensionName)};

		m_queueFamilyIndices = selectQueueFamilyIndices(**m_physicalDevice, use_present);

		if (m_queueFamilyIndices.graphics == UINT32_MAX)
			TST_PERMA_ASSERT_MSG(false, "Failed to find a graphics queue family");

		constexpr float32 default_queue_priority{1.0f};

		std::array<vk::DeviceQueueCreateInfo, 1u> queue_create_infos{};
		queue_create_infos[0].queueFamilyIndex = m_queueFamilyIndices.graphics;
		queue_create_infos[0].queueCount       = 1u;
		queue_create_infos[0].pQueuePriorities = &default_queue_priority;

		const auto enabled_extensions_vec{p_spec_info.enabledExtensions | std::ranges::to<std::vector>()};

		vk::DeviceCreateInfo logical_device_create_info{};
		logical_device_create_info.queueCreateInfoCount    = queue_create_infos.size();
		logical_device_create_info.pQueueCreateInfos       = queue_create_infos.data();
		logical_device_create_info.enabledExtensionCount   = enabled_extensions_vec.size();
		logical_device_create_info.ppEnabledExtensionNames = enabled_extensions_vec.data();
		logical_device_create_info.pNext                   = p_spec_info.pNextDeviceFeatures;

		m_logicalDevice = (**m_physicalDevice).createDevice(logical_device_create_info);

		m_graphicsQueue = m_logicalDevice.getQueue(m_queueFamilyIndices.graphics, 0u);

		vk::CommandPoolCreateInfo graphics_command_pool_create_info{};
		graphics_command_pool_create_info.queueFamilyIndex = m_queueFamilyIndices.graphics;
		graphics_command_pool_create_info.flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
		m_graphicsCommandPool                              = m_logicalDevice.createCommandPool(graphics_command_pool_create_info);
	}

	LogicalDevice::~LogicalDevice()
	{
		m_logicalDevice.destroyCommandPool(m_graphicsCommandPool);

		m_logicalDevice.destroy();
	}
}
