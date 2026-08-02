#pragma once

#include "physical_device.hpp"

namespace toaster::gpu
{
	struct TST_GPU_API LogicalDeviceSpecInfo
	{
		ExtensionSet enabledExtensions; // These are DEVICE extensions

		void *pNextDeviceFeatures{nullptr}; // Use for chaining device features
	};

	struct TST_GPU_API QueueFamilyIndices
	{
		uint32 graphics{UINT32_MAX};
		uint32 present{UINT32_MAX};
		uint32 compute{UINT32_MAX};
	};

	TST_GPU_API auto selectQueueFamilyIndices(vk::PhysicalDevice p_physical_device, bool p_use_present) -> QueueFamilyIndices;

	class TST_GPU_API LogicalDevice
	{
	public:
		LogicalDevice(PhysicalDevice &p_physical_device, const LogicalDeviceSpecInfo &p_spec_info);

		// You cannot modify a physical device, so it is always const
		auto getPhysicalDevice() const -> const PhysicalDevice & { return *m_physicalDevice; }

		auto getVulkanLogicalDevice() const -> const vk::raii::Device & { return m_logicalDevice; }
		auto getVulkanLogicalDevice() -> vk::raii::Device & { return m_logicalDevice; }
		auto operator *() const -> const vk::raii::Device & { return m_logicalDevice; }
		auto operator *() -> vk::raii::Device & { return m_logicalDevice; }

		auto getQueueFamilyIndices() const -> const QueueFamilyIndices & { return m_queueFamilyIndices; }

		// For now this is also the present and compute queue as well
		auto getGraphicsQueue() -> vk::raii::Queue & { return m_graphicsQueue; }

		auto getGraphicsCommandPool() -> vk::raii::CommandPool & { return m_graphicsCommandPool; }

	private:
		// A logical device should know what physical device it is associated with
		PhysicalDevice *m_physicalDevice{nullptr};

		vk::raii::Device m_logicalDevice{nullptr};

		QueueFamilyIndices m_queueFamilyIndices{};
		vk::raii::Queue    m_graphicsQueue{nullptr};

		vk::raii::CommandPool m_graphicsCommandPool{nullptr};
	};
}
