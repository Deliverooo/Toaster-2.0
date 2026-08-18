#include "toast_gpu/buffer.hpp"

namespace toaster::gpu
{
	BufferManager::BufferManager(LogicalDevice &p_device, Allocator &p_allocator, ResourceDescriptorHeap &p_resource_heap) : m_device(&p_device),
																															 m_allocator(&p_allocator),
																															 m_resourceHeap(&p_resource_heap)
	{
	}

	BufferManager::~BufferManager()
	{
	}

	auto BufferManager::createBuffer(const BufferDesc &p_desc) -> BufferHandle
	{
		BufferData buffer_data{};
		buffer_data.usageFlags       = p_desc.usageFlags;
		buffer_data.size             = p_desc.size;
		buffer_data.memoryProperties = p_desc.memoryProperties;

		VmaAllocationCreateFlags allocation_create_flags{
			(buffer_data.memoryProperties == EMemoryProperties::eHostVisibleCoherent) ? VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT : 0u
		};

		m_allocator->createBuffer(buffer_data.size, buffer_data.usageFlags, allocation_create_flags, buffer_data.buffer, buffer_data.allocation);

		// If the buffer is host visible, map it's memory
		if (buffer_data.memoryProperties == EMemoryProperties::eHostVisibleCoherent)
			vmaMapMemory(m_allocator->getAllocator(), buffer_data.allocation, &buffer_data.mapped);

		return m_pool.create(buffer_data);
	}

	auto BufferManager::destroyBuffer(BufferHandle p_handle) -> void
	{
		if (!m_pool.isValid(p_handle))
			TST_PERMA_ASSERT(false);

		m_pool.destroy(p_handle);
		BufferData *data{&m_pool._data[p_handle.id]};

		// Unmap the buffer's memory
		if (data->mapped && data->memoryProperties == EMemoryProperties::eHostVisibleCoherent)
			vmaUnmapMemory(m_allocator->getAllocator(), data->allocation);

		if (data->buffer && data->allocation)
		{
			vmaDestroyBuffer(m_allocator->getAllocator(), data->buffer, data->allocation);
			data->buffer     = nullptr;
			data->allocation = nullptr;
		}
	}

	auto BufferManager::setBufferData(BufferHandle p_handle, vk::CommandBuffer p_cmd, uint64 p_target_semaphore_value, const void *p_data, uint64 p_size,
									  uint64       p_offset) -> void
	{
		BufferData *dst_buffer{getData(p_handle)};
		TST_PERMA_ASSERT(dst_buffer && dst_buffer->buffer && dst_buffer->allocation);

		if (dst_buffer->memoryProperties == EMemoryProperties::eHostVisibleCoherent)
		{
			std::memcpy(dst_buffer->mapped, p_data, p_size);
			return;
		}

		BufferDesc staging_buffer_desc{};
		staging_buffer_desc.size             = p_size;
		staging_buffer_desc.usageFlags       = vk::BufferUsageFlagBits::eTransferSrc;
		staging_buffer_desc.memoryProperties = EMemoryProperties::eHostVisibleCoherent;

		BufferHandle staging_buffer{createBuffer(staging_buffer_desc)};
		BufferData * staging_data{getData(staging_buffer)};

		dst_buffer = getData(p_handle); // FIX: Due to vector reallocation when creating the staging buffer, the dst one becomes invalid

		std::memcpy(staging_data->mapped, p_data, p_size);

		vk::BufferCopy2     buffer_copy{0u, p_offset, p_size};
		vk::CopyBufferInfo2 copy_buffer_info{};
		copy_buffer_info.srcBuffer = staging_data->buffer;
		copy_buffer_info.dstBuffer = dst_buffer->buffer;
		copy_buffer_info.setRegions(buffer_copy);
		p_cmd.copyBuffer2(copy_buffer_info);

		m_deferredStagingDestructions.push_back({p_target_semaphore_value, staging_buffer});
	}

	auto BufferManager::isValid(BufferHandle p_handle) const -> bool
	{
		return m_pool.isValid(p_handle);
	}

	auto BufferManager::getData(BufferHandle p_handle) -> BufferData *
	{
		return m_pool.getData(p_handle);
	}

	auto BufferManager::processDeferredDestructions(uint64 p_current_semaphore_value) -> void
	{
		if (m_deferredStagingDestructions.empty())
			return;

		for (auto it{m_deferredStagingDestructions.begin()}; it != m_deferredStagingDestructions.end();)
		{
			if (p_current_semaphore_value >= it->targetValue)
			{
				destroyBuffer(it->buffer);
				it = m_deferredStagingDestructions.erase(it);
			}
			else
				++it;
		}
	}
}
