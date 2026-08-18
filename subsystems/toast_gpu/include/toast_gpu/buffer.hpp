#pragma once

#include "descriptor_heap.hpp"
#include "gpu_common.hpp"
#include "toast_lib/pool.hpp"

namespace toaster::gpu
{
	struct TST_GPU_API BufferData
	{
		vk::Buffer           buffer{nullptr};
		VmaAllocation        allocation{nullptr};
		vk::DeviceSize       size{0u};
		vk::DeviceAddress    address{0u};
		vk::BufferUsageFlags usageFlags{};
		EMemoryProperties    memoryProperties{EMemoryProperties::eDeviceLocal};

		void *mapped{nullptr}; // nullptr if the buffer is not host visible /coherent

		DescriptorSlot heapID{invalidBufferDescriptorSlot}; // Invalid unless you are using this as a descriptor
	};

	TST_DECLARE_HANDLE(Buffer);

	struct TST_GPU_API BufferDesc
	{
		vk::DeviceSize       size{0u};
		vk::BufferUsageFlags usageFlags{};
		EMemoryProperties    memoryProperties;
	};

	class TST_GPU_API BufferManager
	{
	public:
		BufferManager(LogicalDevice &p_device, Allocator &p_allocator, ResourceDescriptorHeap &p_resource_heap);
		~BufferManager();

		[[nodiscard]] auto createBuffer(const BufferDesc &p_desc) -> BufferHandle;
		auto               destroyBuffer(BufferHandle p_handle) -> void;

		auto setBufferData(BufferHandle p_handle, vk::CommandBuffer p_cmd, uint64 p_target_semaphore_value, const void *p_data, uint64 p_size,
						   uint64       p_offset = 0u) -> void;

		auto isValid(BufferHandle p_handle) const -> bool;
		auto getData(BufferHandle p_handle) -> BufferData *;

		auto processDeferredDestructions(uint64 p_current_semaphore_value) -> void;

	private:
		NonOwningPtr<LogicalDevice>          m_device{nullptr};
		NonOwningPtr<Allocator>              m_allocator{nullptr};
		NonOwningPtr<ResourceDescriptorHeap> m_resourceHeap{nullptr};

		struct DeferredStagingData
		{
			uint64       targetValue;
			BufferHandle buffer;
		};

		std::vector<DeferredStagingData> m_deferredStagingDestructions;

		Pool<BufferTag, BufferData> m_pool;
	};
}
