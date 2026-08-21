#include "toast_gpu/buffer.hpp"

namespace toaster::gpu
{
	BufferManager::BufferManager(LogicalDevice &p_device, Allocator &p_allocator, void *p_user_data, const DestroyCallback &p_destroy_callback) : m_device(&p_device),
																																				  m_allocator(&p_allocator),
																																				  m_userData(p_user_data),
																																				  m_destroyCallback(p_destroy_callback)
	{
		m_pool.setUserData(this);
		m_pool.setDestroyCallback(+[](void *p_user_data, BufferHandle p_handle) -> void
		{
			auto ts{static_cast<BufferManager *>(p_user_data)};

			BufferData *buffer_data{&ts->m_pool._data[p_handle.id]};

			if (ts->m_destroyCallback)
				ts->m_destroyCallback(ts->m_userData, p_handle);
			else
				ts->destroyData(buffer_data);
		});
	}

	BufferManager::~BufferManager()
	{
		// For safety...
		for (uint32 i{0u}; i < m_pool.getSize(); ++i)
		{
			if (m_pool._alive[i])
				destroyData(&m_pool._data[i]);
		}
	}

	auto BufferManager::createBuffer(const BufferDesc &p_desc) -> BufferHandle
	{
		BufferData buffer_data{};
		buffer_data.usageFlags       = p_desc.usageFlags;
		buffer_data.size             = p_desc.size;
		buffer_data.memoryProperties = p_desc.memoryProperties;

		if (buffer_data.memoryProperties == EMemoryProperties::eDeviceLocal)
			m_allocator->createBuffer(buffer_data.size, buffer_data.usageFlags, 0u, buffer_data.buffer, buffer_data.allocation);
		else if (buffer_data.memoryProperties == EMemoryProperties::eHostVisibleCoherent)
		{
			m_allocator->createBuffer(buffer_data.size, buffer_data.usageFlags, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
									  buffer_data.buffer, buffer_data.allocation, &buffer_data.mapped);
		}
		else
			TST_PERMA_ASSERT_MSG(false, "What is dis?");

		if (p_desc.usageFlags & vk::BufferUsageFlagBits::eShaderDeviceAddress)
			buffer_data.address = m_device->getDevice().getBufferAddress({buffer_data.buffer});

		return m_pool.create(buffer_data);
	}

	auto BufferManager::createSharedBuffer(const BufferDesc &p_desc) -> SharedHandle<BufferTag, BufferData>
	{
		return {createBuffer(p_desc), &m_pool};
	}

	auto BufferManager::destroyData(BufferData *p_data) -> void
	{
		if (p_data->buffer && p_data->allocation)
		{
			vmaDestroyBuffer(m_allocator->getAllocator(), p_data->buffer, p_data->allocation);
			p_data->buffer     = nullptr;
			p_data->allocation = nullptr;
		}
	}

	auto BufferManager::uploadDirect(BufferHandle p_handle, const void *p_data, uint64 p_size, uint64 p_offset) -> void
	{
		BufferData *dst_buffer{m_pool.getData(p_handle)};
		TST_PERMA_ASSERT(dst_buffer && dst_buffer->buffer && dst_buffer->allocation && dst_buffer->memoryProperties == EMemoryProperties::eHostVisibleCoherent);

		std::memcpy(reinterpret_cast<void *>(reinterpret_cast<uint64>(dst_buffer->mapped) + p_offset), p_data, p_size);
	}
}
