#include "vk_index_buffer.hpp"

#include "vk_gpu_context.hpp"

namespace toaster::gpu
{
	VKIndexBuffer::VKIndexBuffer(VKGPUContext *p_ctx, void *p_data, uint64 p_size) : m_ctx(p_ctx)
	{
		TST_ASSERT_MSG(p_ctx, "Context cannot be null");

		vk::raii::Buffer       staging_buffer{nullptr};
		vk::raii::DeviceMemory staging_buffer_memory{nullptr};

		m_ctx->createBuffer(p_size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible,
							staging_buffer, staging_buffer_memory);

		void *mapped = staging_buffer_memory.mapMemory(0, p_size, {});
		std::memcpy(mapped, p_data, p_size);
		staging_buffer_memory.unmapMemory();

		m_ctx->createBuffer(p_size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal,
							m_indexBuffer, m_indexBufferMemory);
		m_ctx->copyBuffer(staging_buffer, m_indexBuffer, p_size);
	}

	VKIndexBuffer::VKIndexBuffer(VKGPUContext *p_ctx, uint64 p_size) : m_ctx(p_ctx)
	{
		m_ctx->createBuffer(p_size, vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible,
							m_indexBuffer, m_indexBufferMemory);
	}

	vk::raii::Buffer &VKIndexBuffer::getBuffer()
	{
		return m_indexBuffer;
	}

	vk::raii::DeviceMemory &VKIndexBuffer::getBufferMemory()
	{
		return m_indexBufferMemory;
	}

	void VKIndexBuffer::setData(void *p_data, uint64 p_size, uint64 p_offset)
	{
		void *mapped = m_indexBufferMemory.mapMemory(p_offset, p_size, {});
		std::memcpy(mapped, p_data, p_size);
		m_indexBufferMemory.unmapMemory();
	}

	void VKIndexBuffer::bind(vk::raii::CommandBuffer &p_command_buffer, vk::IndexType p_index_type)
	{
		p_command_buffer.bindIndexBuffer(m_indexBuffer, 0, p_index_type);
	}
}
