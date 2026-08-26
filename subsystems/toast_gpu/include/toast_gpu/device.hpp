#pragma once

#include "allocator.hpp"
#include "deletion_queue.hpp"

namespace toaster::gpu
{
	struct TST_GPU_API DeviceDesc
	{
		bool  enableDebugInfo{true}; // Translates directly to whether validation layers are enabled.
		bool  usingSwapchain{true};  // If true, the GPUContext will use the required instance and device extensions
		uint8 numDeletionQueues{3u}; // Set this to your max frames in flight. (Should be 3 already)
	};

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

		#pragma region deletion queue

		// Submits an arbitrary invokable to the current deletion queue
		template<typename TFunc> requires  std::is_invocable_v<TFunc> && std::is_nothrow_invocable_v<TFunc>
		auto submitDeletion(TFunc &&p_func) -> void
		{
			m_deletionQueue->submit(std::forward<TFunc>(p_func));
		}

		// Flushes the current deletion queue and swaps it
		auto performGarbageCollection(uint32 p_queue_index) -> void; // For the queue index, pass the current frame index...

		// Flushes the entire deletion queue and clears all pending deletions
		auto flushDeletionQueue() -> void;

		#pragma endregion

		auto createTimelineSemaphore(uint64 p_initial_value = 0u) const -> vk::Semaphore;
		auto waitForTimelineSemaphores(const InitialiserList<const vk::Semaphore> &p_semaphores, const InitialiserList<const uint64> &p_target_values) const -> void;

		auto getBufferAddress(vk::Buffer p_buffer) const -> vk::DeviceAddress { return m_device->getDevice().getBufferAddress({p_buffer}); }

	private:
		UniquePtr<Instance>       m_instance{nullptr};
		UniquePtr<PhysicalDevice> m_physicalDevice{nullptr};
		UniquePtr<LogicalDevice>  m_device{nullptr};

		UniquePtr<Allocator>     m_allocator{nullptr};
		UniquePtr<DeletionQueue> m_deletionQueue;
	};
}
