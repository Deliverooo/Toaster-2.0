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

	EResourceType VKUniformBuffer::getResourceType() const
	{
		return EResourceType::eUniformBuffer;
	}

	VKUniformBufferPFF::VKUniformBufferPFF(VKGPUContext *p_ctx, uint64 p_size, uint32 p_frames_in_flight) : m_framesInFlightCount(p_frames_in_flight)
	{
		TST_ASSERT_MSG(p_ctx, "Context cannot be null");
		TST_ASSERT_MSG(p_frames_in_flight > 0, "Frames in flight cannot be 0");

		for (uint32 i{0u}; i < m_framesInFlightCount; ++i)
			m_uniformBuffers.emplace_back(make_reference<VKUniformBuffer>(p_ctx, p_size));
	}

	RefPtr<VKUniformBuffer> VKUniformBufferPFF::getUBO(uint32 p_frame_index)
	{
		TST_ASSERT_MSG(p_frame_index < m_framesInFlightCount, "Out of range");
		return m_uniformBuffers[p_frame_index];
	}

	void VKUniformBufferPFF::setUBO(uint32 p_frame_index, const RefPtr<VKUniformBuffer> &p_uniform_buffer)
	{
		TST_ASSERT_MSG(p_frame_index < m_framesInFlightCount, "Out of range");
		m_uniformBuffers[p_frame_index] = p_uniform_buffer;
	}

	std::vector<RefPtr<VKUniformBuffer> >::iterator VKUniformBufferPFF::begin()
	{
		return m_uniformBuffers.begin();
	}

	std::vector<RefPtr<VKUniformBuffer> >::iterator VKUniformBufferPFF::end()
	{
		return m_uniformBuffers.end();
	}

	EResourceType VKUniformBufferPFF::getResourceType() const
	{
		return EResourceType::eUniformBufferPFF;
	}

	std::vector<void *> VKUniformBufferPFF::mapMemory(uint64 p_size, uint64 p_offset)
	{
		std::vector<void *> mapped{};
		for (auto &ubo: m_uniformBuffers)
			mapped.emplace_back(ubo->mapMemory(p_size, p_offset));
		return mapped;
	}

	void VKUniformBufferPFF::unmapMemory()
	{
		for (auto &ubo: m_uniformBuffers)
			ubo->unmapMemory();
	}
}
