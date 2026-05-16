#include "vk_vertex_buffer.hpp"

#include "vk_logical_device.hpp"

namespace toaster::gpu
{
	VKVertexBuffer::VKVertexBuffer(VKLogicalDevice *p_device, void *p_data, uint64 p_size) : m_device(p_device)
	{
		TST_ASSERT_MSG(p_device, "Device cannot be null");

		vk::raii::Buffer       staging_buffer{nullptr};
		vk::raii::DeviceMemory staging_buffer_memory{nullptr};

		m_device->createBuffer(p_size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible,
							   staging_buffer, staging_buffer_memory);

		void *mapped = staging_buffer_memory.mapMemory(0u, p_size, {});
		std::memcpy(mapped, p_data, p_size);
		staging_buffer_memory.unmapMemory();

		m_device->createBuffer(p_size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal,
							   m_vertexBuffer, m_vertexBufferMemory);
		m_device->copyBuffer(*staging_buffer, m_vertexBuffer, p_size);
	}

	VKVertexBuffer::VKVertexBuffer(VKLogicalDevice *p_device, uint64 p_size) : m_device(p_device)
	{
		m_device->createBuffer(p_size, vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible,
							   m_vertexBuffer, m_vertexBufferMemory);
	}

	VKVertexBuffer::~VKVertexBuffer()
	{
		m_device->destroy(m_vertexBuffer);
		m_device->destroy(m_vertexBufferMemory);
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

	auto VKVertexBuffer::bind(const vk::raii::CommandBuffer &p_command_buffer) -> void
	{
		p_command_buffer.bindVertexBuffers(0, m_vertexBuffer, {0});
	}
}
