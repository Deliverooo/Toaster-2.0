#include "vk_logical_device.hpp"

namespace toaster::gpu
{
	VKLogicalDevice::VKLogicalDevice(VKPhysicalDevice *p_physical_device, const VKLogicalDeviceSpecInfo &p_spec_info) : m_physicalDevice(p_physical_device),
																														m_specInfo(p_spec_info)
	{
		const bool using_present{m_specInfo.surface != nullptr};

		const auto queue_family_props = m_physicalDevice->getVulkanPhysicalDevice().getQueueFamilyProperties();

		for (uint32 i{0u}; i < queue_family_props.size(); ++i)
		{
			if (queue_family_props[i].queueFlags & vk::QueueFlagBits::eGraphics && using_present
					? (m_physicalDevice->getVulkanPhysicalDevice().getSurfaceSupportKHR(i, m_specInfo.surface))
					: true)
			{
				m_queueFamilyIndices.graphics = i;
				break;
			}
		}
		for (uint32 i{0u}; i < queue_family_props.size(); ++i)
		{
			if (queue_family_props[i].queueFlags & vk::QueueFlagBits::eCompute && queue_family_props[i].queueFlags & vk::QueueFlagBits::eGraphics)
			{
				m_queueFamilyIndices.compute = i;
				break;
			}
		}
		for (uint32 i{0u}; i < queue_family_props.size(); ++i)
		{
			if (queue_family_props[i].queueFlags & vk::QueueFlagBits::eTransfer && (
					static_cast<uint32>(queue_family_props[i].queueFlags & vk::QueueFlagBits::eGraphics) == 0) && (
					static_cast<uint32>(queue_family_props[i].queueFlags & vk::QueueFlagBits::eCompute) == 0))
			{
				m_queueFamilyIndices.transfer = i;
				break;
			}
		}

		if (m_queueFamilyIndices.graphics == UINT32_MAX)
		{
			LOG_ERROR("Failed to find a queue family that supports present");
			TST_ASSERT(false);
		}

		if (m_queueFamilyIndices.transfer == UINT32_MAX)
		{
			LOG_ERROR("Failed to find a transfer queue family");
			TST_ASSERT(false);
		}

		if (m_queueFamilyIndices.compute == UINT32_MAX)
		{
			LOG_ERROR("Failed to find a compute queue family");
			TST_ASSERT(false);
		}

		std::vector<vk::DeviceQueueCreateInfo> queue_create_infos{};

		bool has_separate_compute_queue = m_queueFamilyIndices.graphics != m_queueFamilyIndices.compute;

		// Create the graphics and present queue
		// The graphics queue should be the same as the present one
		auto &graphics_queue_create_info            = queue_create_infos.emplace_back();
		graphics_queue_create_info.queueFamilyIndex = m_queueFamilyIndices.graphics;
		graphics_queue_create_info.queueCount       = has_separate_compute_queue ? 1 : 2;
		std::vector<float32> queue_priorities;
		queue_priorities.emplace_back(1.0f);
		if (!has_separate_compute_queue)
			queue_priorities.emplace_back(1.0f);
		graphics_queue_create_info.pQueuePriorities = queue_priorities.data();

		constexpr float32 queue_priority = 1.0f;

		// Create the transfer queue
		auto &transfer_queue_create_info            = queue_create_infos.emplace_back();
		transfer_queue_create_info.queueFamilyIndex = m_queueFamilyIndices.transfer;
		transfer_queue_create_info.queueCount       = 1;
		transfer_queue_create_info.pQueuePriorities = &queue_priority;

		if (has_separate_compute_queue)
		{
			// Create the compute queue
			auto &compute_queue_create_info            = queue_create_infos.emplace_back();
			compute_queue_create_info.queueFamilyIndex = m_queueFamilyIndices.compute;
			compute_queue_create_info.queueCount       = 1;
			compute_queue_create_info.pQueuePriorities = &queue_priority;
		}

		const std::vector<CString> extension_vec{m_specInfo.requiredExtensions.begin(), m_specInfo.requiredExtensions.end()};
		vk::DeviceCreateInfo       device_create_info{};
		device_create_info.enabledExtensionCount   = static_cast<uint32>(extension_vec.size());
		device_create_info.ppEnabledExtensionNames = extension_vec.data();
		device_create_info.queueCreateInfoCount    = queue_create_infos.size();
		device_create_info.pQueueCreateInfos       = queue_create_infos.data();
		device_create_info.pNext                   = m_specInfo.pNext;

		m_logicalDevice = {m_physicalDevice->getVulkanPhysicalDevice(), device_create_info};

		// Create the queues
		m_graphicsQueue = {m_logicalDevice, m_queueFamilyIndices.graphics, 0};
		m_transferQueue = {m_logicalDevice, m_queueFamilyIndices.transfer, 0};
		if (m_queueFamilyIndices.graphics == m_queueFamilyIndices.compute)
			m_computeQueue = {m_logicalDevice, m_queueFamilyIndices.compute, 1};

		LOG_TRACE("Graphics queue family index {}", m_queueFamilyIndices.graphics);
		LOG_TRACE("Transfer queue family index {}", m_queueFamilyIndices.transfer);
		LOG_TRACE("Compute queue family index {}", m_queueFamilyIndices.compute);

		#pragma region create command pools
		// Graphics
		vk::CommandPoolCreateInfo graphics_command_pool_create_info{};
		graphics_command_pool_create_info.queueFamilyIndex = m_queueFamilyIndices.graphics;
		graphics_command_pool_create_info.flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

		m_graphicsCommandPool = {m_logicalDevice, graphics_command_pool_create_info};

		// Transfer
		vk::CommandPoolCreateInfo transfer_command_pool_create_info{};
		transfer_command_pool_create_info.queueFamilyIndex = m_queueFamilyIndices.transfer;
		transfer_command_pool_create_info.flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

		m_transferCommandPool = {m_logicalDevice, transfer_command_pool_create_info};

		// Compute
		vk::CommandPoolCreateInfo compute_command_pool_create_info{};
		compute_command_pool_create_info.queueFamilyIndex = m_queueFamilyIndices.compute;
		compute_command_pool_create_info.flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

		m_computeCommandPool = {m_logicalDevice, compute_command_pool_create_info};

		#pragma endregion
	}

	auto VKLogicalDevice::getPhysicalDevice() const -> VKPhysicalDevice *
	{
		return m_physicalDevice;
	}

	auto VKLogicalDevice::getSpecInfo() const -> const VKLogicalDeviceSpecInfo &
	{
		return m_specInfo;
	}

	auto VKLogicalDevice::getVulkanLogicalDevice() -> vk::raii::Device &
	{
		return m_logicalDevice;
	}

	auto VKLogicalDevice::getQueueFamilyIndices() const -> const QueueFamilyIndices &
	{
		return m_queueFamilyIndices;
	}

	auto VKLogicalDevice::getGraphicsQueue() -> vk::raii::Queue &
	{
		return m_graphicsQueue;
	}

	auto VKLogicalDevice::getTransferQueue() -> vk::raii::Queue &
	{
		return m_transferQueue;
	}

	auto VKLogicalDevice::getComputeQueue() -> vk::raii::Queue &
	{
		return m_computeQueue;
	}

	auto VKLogicalDevice::getGraphicsCommandPool() -> vk::raii::CommandPool &
	{
		return m_graphicsCommandPool;
	}

	auto VKLogicalDevice::getTransferCommandPool() -> vk::raii::CommandPool &
	{
		return m_transferCommandPool;
	}

	auto VKLogicalDevice::getComputeCommandPool() -> vk::raii::CommandPool &
	{
		return m_computeCommandPool;
	}

	auto VKLogicalDevice::waitForFence(const vk::Fence &p_fence, uint64 p_timeout) const -> void
	{
		if (const vk::Result fence_result{m_logicalDevice.waitForFences({p_fence}, true, p_timeout)}; fence_result != vk::Result::eSuccess)
		{
			TST_ASSERT_MSG(false, "Failed to wait for Fence");
		}
	}

	auto VKLogicalDevice::waitForFences(const std::initializer_list<const vk::Fence> &p_fences, bool p_wait_all, uint64 p_timeout) const -> void
	{
		if (const vk::Result fence_result{m_logicalDevice.waitForFences({p_fences}, p_wait_all, p_timeout)}; fence_result != vk::Result::eSuccess)
		{
			TST_ASSERT_MSG(false, "Failed to wait for Fences");
		}
	}
}
