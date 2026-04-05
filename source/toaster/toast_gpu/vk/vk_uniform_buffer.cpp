#include "vk_uniform_buffer.hpp"

#include "vk_gpu_context.hpp"

namespace toaster::gpu
{
	VKUniformBuffer::VKUniformBuffer(VKGPUContext *p_ctx, uint64 p_size) : m_ctx(p_ctx)
	{
		TST_ASSERT_MSG(p_ctx, "Context cannot be null");

		m_ctx->createBuffer(p_size, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
							m_buffer, m_bufferMemory);

		m_descriptorInfo.buffer = m_buffer;
		m_descriptorInfo.offset = 0;
		m_descriptorInfo.range  = p_size;
	}

	vk::raii::Buffer &VKUniformBuffer::getBuffer()
	{
		return m_buffer;
	}

	vk::raii::DeviceMemory &VKUniformBuffer::getBufferMemory()
	{
		return m_bufferMemory;
	}

	const vk::DescriptorBufferInfo &VKUniformBuffer::getDescriptorInfo() const
	{
		return m_descriptorInfo;
	}

	void VKUniformBuffer::setData(void *p_data, uint64 p_size, uint64 p_offset)
	{
		void *mapped = m_bufferMemory.mapMemory(p_offset, p_size);
		std::memcpy(mapped, p_data, p_size);
		m_bufferMemory.unmapMemory();
	}

	void *VKUniformBuffer::mapMemory(uint64 p_size, uint64 p_offset)
	{
		return m_bufferMemory.mapMemory(p_offset, p_size);
	}

	void VKUniformBuffer::unmapMemory()
	{
		m_bufferMemory.unmapMemory();
	}
}
