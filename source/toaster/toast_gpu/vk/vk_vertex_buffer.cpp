#include "vk_vertex_buffer.hpp"

#include "vk_gpu_context.hpp"

namespace toaster::gpu
{
	VKVertexBuffer::VKVertexBuffer(VKGPUContext *p_ctx, void *p_data, uint64 p_size) : m_ctx(p_ctx)
	{
		TST_ASSERT_MSG(p_ctx, "Context cannot be null");

		vk::raii::Buffer       staging_buffer{nullptr};
		vk::raii::DeviceMemory staging_buffer_memory{nullptr};

		m_ctx->createBuffer(p_size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible,
							staging_buffer, staging_buffer_memory);

		void *mapped = staging_buffer_memory.mapMemory(0, p_size, {});
		std::memcpy(mapped, p_data, p_size);
		staging_buffer_memory.unmapMemory();

		m_ctx->createBuffer(p_size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal,
							m_vertexBuffer, m_vertexBufferMemory);
		m_ctx->copyBuffer(staging_buffer, m_vertexBuffer, p_size);
	}

	VKVertexBuffer::VKVertexBuffer(VKGPUContext *p_ctx, uint64 p_size) : m_ctx(p_ctx)
	{
		m_ctx->createBuffer(p_size, vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible,
							m_vertexBuffer, m_vertexBufferMemory);
	}

	vk::raii::Buffer &VKVertexBuffer::getBuffer()
	{
		return m_vertexBuffer;
	}

	vk::raii::DeviceMemory &VKVertexBuffer::getBufferMemory()
	{
		return m_vertexBufferMemory;
	}

	void VKVertexBuffer::setData(void *p_data, uint64 p_size, uint64 p_offset)
	{
		void *mapped = m_vertexBufferMemory.mapMemory(p_offset, p_size, {});
		std::memcpy(mapped, p_data, p_size);
		m_vertexBufferMemory.unmapMemory();
	}

	void VKVertexBuffer::bind(vk::raii::CommandBuffer &p_command_buffer)
	{
		p_command_buffer.bindVertexBuffers(0, *m_vertexBuffer, {0});
	}
}
