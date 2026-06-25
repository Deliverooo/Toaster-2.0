#include "toast_gpu/vk/vk_vertex_buffer.hpp"
#include "toast_gpu/vk/vk_logical_device.hpp"

namespace toaster::gpu
{
	VKVertexBuffer::VKVertexBuffer(VKLogicalDevice *p_device, const void *p_data, uint64 p_size) : m_device(p_device)
	{
		TST_ASSERT_MSG(p_device, "Device cannot be null");

		vk::raii::Buffer       staging_buffer{nullptr};
		vk::raii::DeviceMemory staging_buffer_memory{nullptr};

		m_device->createBuffer(staging_buffer, staging_buffer_memory, p_size, vk::BufferUsageFlagBits2::eTransferSrc,
							   vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);

		void *mapped = staging_buffer_memory.mapMemory(0u, p_size, {});
		std::memcpy(mapped, p_data, p_size);
		staging_buffer_memory.unmapMemory();

		m_device->createBuffer(m_vertexBuffer, m_vertexBufferMemory, p_size, vk::BufferUsageFlagBits2::eTransferDst | vk::BufferUsageFlagBits2::eVertexBuffer,
							   vk::MemoryPropertyFlagBits::eDeviceLocal);
		m_device->copyBuffer(*staging_buffer, m_vertexBuffer, p_size);
	}

	VKVertexBuffer::VKVertexBuffer(VKLogicalDevice *p_device, uint64 p_size) : m_device(p_device)
	{
		m_device->createBuffer(m_vertexBuffer, m_vertexBufferMemory, p_size, vk::BufferUsageFlagBits2::eVertexBuffer,
							   vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);
	}

	VKVertexBuffer::~VKVertexBuffer()
	{
		m_device->deferDestruction([device = m_device, buffer = m_vertexBuffer, buffer_memory = m_vertexBufferMemory]() mutable-> void
		{
			device->destroyObject(buffer);
			device->destroyObject(buffer_memory);
		});
	}

	auto VKVertexBuffer::getBuffer() -> vk::Buffer &
	{
		return m_vertexBuffer;
	}

	auto VKVertexBuffer::getBufferMemory() -> vk::DeviceMemory &
	{
		return m_vertexBufferMemory;
	}

	auto VKVertexBuffer::setData(void *p_data, uint64 p_size, uint64 p_offset) -> void
	{
		void *mapped = m_device->mapMemory(m_vertexBufferMemory, p_offset, p_size, {});
		std::memcpy(mapped, p_data, p_size);
		m_device->unmapMemory(m_vertexBufferMemory);
	}

	auto VKVertexBuffer::bind(VKCommandBuffer *p_command_buffer) -> void
	{
		TST_GPU_GET_VALID_CMD_BUFFER();
		cmd->getVulkanCommandBuffer().bindVertexBuffers(0, m_vertexBuffer, {0});
	}
}
