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

	auto VKUniformBuffer::getContext() const -> VKGPUContext *
	{
		return m_ctx;
	}

	auto VKUniformBuffer::getBuffer() -> vk::raii::Buffer &
	{
		return m_buffer;
	}

	auto VKUniformBuffer::getBufferMemory() -> vk::raii::DeviceMemory &
	{
		return m_bufferMemory;
	}

	auto VKUniformBuffer::getDescriptorInfo() const -> const vk::DescriptorBufferInfo &
	{
		return m_descriptorInfo;
	}

	auto VKUniformBuffer::setData(void *p_data, uint64 p_size, uint64 p_offset) -> void
	{
		void *mapped = m_bufferMemory.mapMemory(p_offset, p_size);
		std::memcpy(mapped, p_data, p_size);
		m_bufferMemory.unmapMemory();
	}

	auto VKUniformBuffer::mapMemory(uint64 p_size, uint64 p_offset) -> void *
	{
		return m_bufferMemory.mapMemory(p_offset, p_size);
	}

	auto VKUniformBuffer::unmapMemory() -> void
	{
		m_bufferMemory.unmapMemory();
	}

	auto VKUniformBuffer::getResourceType() const -> EGPUResourceType
	{
		return EGPUResourceType::eUniformBuffer;
	}

	VKUniformBufferPFF::VKUniformBufferPFF(VKGPUContext *p_ctx, uint64 p_size, uint32 p_frames_in_flight) : m_framesInFlightCount(p_frames_in_flight)
	{
		TST_ASSERT_MSG(p_ctx, "Context cannot be null");
		TST_ASSERT_MSG(p_frames_in_flight > 0, "Frames in flight cannot be 0");

		for (uint32 i{0u}; i < m_framesInFlightCount; ++i)
			m_uniformBuffers.emplace_back(p_ctx->alloc<VKUniformBuffer>(p_size));
	}

	auto VKUniformBufferPFF::getUBO(uint32 p_frame_index) -> RefPtr<VKUniformBuffer>
	{
		TST_ASSERT_MSG(p_frame_index < m_framesInFlightCount, "Out of range");
		return m_uniformBuffers[p_frame_index];
	}

	auto VKUniformBufferPFF::setUBO(uint32 p_frame_index, const RefPtr<VKUniformBuffer> &p_uniform_buffer) -> void
	{
		TST_ASSERT_MSG(p_frame_index < m_framesInFlightCount, "Out of range");
		m_uniformBuffers[p_frame_index] = p_uniform_buffer;
	}

	auto VKUniformBufferPFF::begin() -> std::vector<RefPtr<VKUniformBuffer> >::iterator
	{
		return m_uniformBuffers.begin();
	}

	auto VKUniformBufferPFF::end() -> std::vector<RefPtr<VKUniformBuffer> >::iterator
	{
		return m_uniformBuffers.end();
	}

	auto VKUniformBufferPFF::getResourceType() const -> EGPUResourceType
	{
		return EGPUResourceType::eUniformBufferPFF;
	}

	auto VKUniformBufferPFF::mapMemory(uint64 p_size, uint64 p_offset) -> std::vector<void *>
	{
		std::vector<void *> mapped{};
		for (auto &ubo: m_uniformBuffers)
			mapped.emplace_back(ubo->mapMemory(p_size, p_offset));
		return mapped;
	}

	auto VKUniformBufferPFF::unmapMemory() -> void
	{
		for (auto &ubo: m_uniformBuffers)
			ubo->unmapMemory();
	}
}
