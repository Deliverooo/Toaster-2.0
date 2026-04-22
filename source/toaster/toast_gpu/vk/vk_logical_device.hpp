#pragma once
#include "vk_physical_device.hpp"

namespace toaster::gpu
{
	struct VKLogicalDeviceSpecInfo
	{
		using ExtensionSet = std::unordered_set<CString>;

		ExtensionSet requiredExtensions;

		// Optional :)
		vk::SurfaceKHR surface{nullptr};

		// Use ts to set your logical device features using vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features...
		void *pNext{nullptr};
	};

	class VKLogicalDevice
	{
	public:
		struct QueueFamilyIndices
		{
			uint32 graphics{UINT32_MAX};
			uint32 transfer{UINT32_MAX};
			uint32 compute{UINT32_MAX};
		};

		VKLogicalDevice(VKPhysicalDevice *p_physical_device, const VKLogicalDeviceSpecInfo &p_spec_info);

		auto getPhysicalDevice() const -> VKPhysicalDevice *;
		auto getSpecInfo() const -> const VKLogicalDeviceSpecInfo &;

		[[nodiscard]] auto getVulkanLogicalDevice() -> vk::raii::Device &;

		[[nodiscard]] auto getQueueFamilyIndices() const -> const QueueFamilyIndices &;
		[[nodiscard]] auto getGraphicsQueue() -> vk::raii::Queue &;
		[[nodiscard]] auto getTransferQueue() -> vk::raii::Queue &;
		[[nodiscard]] auto getComputeQueue() -> vk::raii::Queue &;

		[[nodiscard]] auto getGraphicsCommandPool() -> vk::raii::CommandPool &;
		[[nodiscard]] auto getTransferCommandPool() -> vk::raii::CommandPool &;
		[[nodiscard]] auto getComputeCommandPool() -> vk::raii::CommandPool &;

		auto waitForFence(const vk::Fence &p_fence, uint64 p_timeout = UINT64_MAX) const -> void;
		auto waitForFences(const std::initializer_list<const vk::Fence> &p_fences, bool p_wait_all = true, uint64 p_timeout = UINT64_MAX) const -> void;

		operator vk::raii::Device &() { return m_logicalDevice; }

	private:
		VKPhysicalDevice *m_physicalDevice{nullptr};

		VKLogicalDeviceSpecInfo m_specInfo{};

		vk::raii::Device m_logicalDevice{nullptr};

		QueueFamilyIndices m_queueFamilyIndices{};
		vk::raii::Queue    m_graphicsQueue{nullptr};
		vk::raii::Queue    m_transferQueue{nullptr};
		vk::raii::Queue    m_computeQueue{nullptr};

		vk::raii::CommandPool m_graphicsCommandPool{nullptr};
		vk::raii::CommandPool m_transferCommandPool{nullptr};
		vk::raii::CommandPool m_computeCommandPool{nullptr};
	};
}
