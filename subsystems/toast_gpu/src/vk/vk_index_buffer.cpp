#include "toast_gpu/vk/vk_index_buffer.hpp"
#include "toast_gpu/vk/vk_logical_device.hpp"

namespace toaster::gpu
{
	VKIndexBuffer::VKIndexBuffer(VKLogicalDevice *p_device, const void *p_data, uint64 p_size) : m_device(p_device)
	{
		TST_ASSERT_MSG(p_device, "Context cannot be null");

		vk::raii::Buffer       staging_buffer{nullptr};
		vk::raii::DeviceMemory staging_buffer_memory{nullptr};

		m_device->createBuffer(p_size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible,
							   staging_buffer, staging_buffer_memory);

		void *mapped = staging_buffer_memory.mapMemory(0, p_size, {});
		std::memcpy(mapped, p_data, p_size);
		staging_buffer_memory.unmapMemory();

		m_device->createBuffer(p_size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal,
							   m_indexBuffer, m_indexBufferMemory);
		m_device->copyBuffer(staging_buffer, m_indexBuffer, p_size);
	}

	VKIndexBuffer::VKIndexBuffer(VKLogicalDevice *p_device, uint64 p_size) : m_device(p_device)
	{
		m_device->createBuffer(p_size, vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible,
							   m_indexBuffer, m_indexBufferMemory);
	}

	VKIndexBuffer::~VKIndexBuffer()
	{
		m_device->deferDestruction([device = m_device, buffer = m_indexBuffer, buffer_memory = m_indexBufferMemory]()mutable -> void
		{
			device->destroyObject(buffer);
			device->destroyObject(buffer_memory);
		});
	}

	auto VKIndexBuffer::getBuffer() -> vk::Buffer &
	{
		return m_indexBuffer;
	}

	auto VKIndexBuffer::getBufferMemory() -> vk::DeviceMemory &
	{
		return m_indexBufferMemory;
	}

	auto VKIndexBuffer::setData(void *p_data, uint64 p_size, uint64 p_offset) -> void
	{
		void *mapped = m_device->mapMemory(m_indexBufferMemory, p_offset, p_size, {});
		std::memcpy(mapped, p_data, p_size);
		m_device->unmapMemory(m_indexBufferMemory);
	}

	auto VKIndexBuffer::bind(VKCommandBuffer *p_command_buffer, vk::IndexType p_index_type) -> void
	{
		TST_GPU_GET_VALID_CMD_BUFFER();
		cmd->getVulkanCommandBuffer().bindIndexBuffer(m_indexBuffer, 0, p_index_type);
	}
}
