#pragma once

#include "physical_device.hpp"

namespace toaster::gpu
{
	struct TST_GPU_API LogicalDeviceDesc
	{
		ExtensionSet enabledExtensions; // These are DEVICE extensions

		void *pNextDeviceFeatures{nullptr}; // Use for chaining device features
	};

	struct TST_GPU_API QueueFamilyIndices
	{
		uint32 graphics{UINT32_MAX};
		uint32 present{UINT32_MAX};
		uint32 compute{UINT32_MAX};
		uint32 transfer{UINT32_MAX}; // Dedicated transfer queue
	};

	TST_GPU_API auto selectQueueFamilyIndices(vk::PhysicalDevice p_physical_device, bool p_use_present) -> QueueFamilyIndices;

	class TST_GPU_API LogicalDevice
	{
	public:
		LogicalDevice(PhysicalDevice &p_physical_device, const LogicalDeviceDesc &p_desc);
		~LogicalDevice();

		auto getDevice() const -> vk::Device { return m_logicalDevice; }

		auto getQueueFamilyIndices() const -> const QueueFamilyIndices & { return m_queueFamilyIndices; }

		auto getGraphicsQueue() -> vk::Queue & { return m_graphicsQueue; } // For now this is also the present and compute queue as well
		auto getTransferQueue() -> vk::Queue & { return m_transferQueue; }

	private:
		vk::Device m_logicalDevice{nullptr};

		QueueFamilyIndices m_queueFamilyIndices{};
		vk::Queue          m_graphicsQueue{nullptr};
		vk::Queue          m_transferQueue{nullptr};
	};
}
