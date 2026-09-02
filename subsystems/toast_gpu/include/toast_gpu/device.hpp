#pragma once

#include "allocator.hpp"
#include "deletion_queue.hpp"

#include "gpu_enums.hpp"

#include "buffer.hpp"
#include "resource_pool.hpp"
#include "texture.hpp"
#include "sampler.hpp"
#include "shader.hpp"

namespace toaster::gpu
{
	struct TST_GPU_API DeviceDesc
	{
		bool  enableDebugInfo{true}; // Translates directly to whether validation layers are enabled.
		bool  usingSwapchain{true};  // If true, the GPUContext will use the required instance and device extensions
		uint8 numDeletionQueues{3u}; // Set this to your max frames in flight. (Should be 3 already)
	};

	class CommandPool;
	class CommandList;

	// Represents the absolute minimum required to set up a Vulkan 'context' and create other objects. And a deletion queue
	class TST_GPU_API Device
	{
	public:
		Device(DeviceDesc p_desc);
		~Device();

		Device(const Device &)            = delete;
		Device(Device &&)                 = delete;
		Device &operator=(const Device &) = delete;
		Device &operator=(Device &&)      = delete;

		auto getInstance() -> Instance & { return *m_instance; }
		auto getInstance() const -> const Instance & { return *m_instance; }

		auto getPhysicalDevice() -> PhysicalDevice & { return *m_physicalDevice; }
		auto getPhysicalDevice() const -> const PhysicalDevice & { return *m_physicalDevice; }

		auto getDevice() -> LogicalDevice & { return *m_device; }
		auto getDevice() const -> const LogicalDevice & { return *m_device; }

		auto getAllocator() -> Allocator & { return *m_allocator; }
		auto getAllocator() const -> const Allocator & { return *m_allocator; }

		auto createCommandPool(EQueueType p_queue_type, ECommandPoolFlags p_pool_flags) -> CommandPool;

		auto executeCommandLists(const InitialiserList<const CommandList> &p_command_lists, const InitialiserList<const vk::SemaphoreSubmitInfo> &p_wait_semaphores = {},
								 const InitialiserList<const vk::SemaphoreSubmitInfo> &p_signal_semaphores = {}, vk::Fence p_signal_fence = nullptr) const -> void;

		[[nodiscard]] auto createTimelineSemaphore(uint64 p_initial_value = 0u) const -> vk::Semaphore;
		auto waitForTimelineSemaphores(const InitialiserList<const vk::Semaphore> &p_semaphores, const InitialiserList<const uint64> &p_target_values) const -> void;

		auto waitIdle() const -> void;

	private:
		UniquePtr<Instance>       m_instance{nullptr};
		UniquePtr<PhysicalDevice> m_physicalDevice{nullptr};
		UniquePtr<LogicalDevice>  m_device{nullptr};

		UniquePtr<Allocator> m_allocator{nullptr};

		friend class CommandList;
	};
}
