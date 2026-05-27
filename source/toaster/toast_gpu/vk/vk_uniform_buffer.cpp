#include "vk_uniform_buffer.hpp"

#include "vk_logical_device.hpp"

namespace toaster::gpu
{
	VKUniformBuffer::VKUniformBuffer(VKLogicalDevice *p_dev, uint64 p_size) : m_device(p_dev)
	{
		TST_ASSERT_MSG(p_dev, "Device cannot be null");

		m_device->createBuffer(p_size, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
							   m_buffer, m_bufferMemory);

		m_descriptorInfo.buffer = m_buffer;
		m_descriptorInfo.offset = 0;
		m_descriptorInfo.range  = p_size;
	}

	VKUniformBuffer::~VKUniformBuffer()
	{
		m_device->deferDestruction([device = m_device, buffer = m_buffer, buffer_memory = m_bufferMemory]() mutable-> void
		{
			device->destroyObject(buffer);
			device->destroyObject(buffer_memory);
		});
	}

	auto VKUniformBuffer::populateWriteDescriptor(vk::WriteDescriptorSet &p_write_descriptor, uint32 p_frame_index) -> void
	{
		p_write_descriptor.pBufferInfo = &m_descriptorInfo;
		if (!p_write_descriptor.pBufferInfo->buffer)
			TST_ASSERT_MSG(false, "Oh no");
	}

	auto VKUniformBuffer::getDescriptorResourceHandle([[maybe_unused]] uint32 p_frame_index) -> void *
	{
		return m_descriptorInfo.buffer;
	}

	auto VKUniformBuffer::getBuffer() -> vk::Buffer &
	{
		return m_buffer;
	}

	auto VKUniformBuffer::getBufferMemory() -> vk::DeviceMemory &
	{
		return m_bufferMemory;
	}

	auto VKUniformBuffer::getDescriptorInfo() const -> const vk::DescriptorBufferInfo &
	{
		return m_descriptorInfo;
	}

	auto VKUniformBuffer::setData(void *p_data, uint64 p_size, uint64 p_offset) -> void
	{
		void *mapped = m_device->mapMemory(m_bufferMemory, p_offset, p_size, {});
		std::memcpy(mapped, p_data, p_size);
		m_device->unmapMemory(m_bufferMemory);
	}

	auto VKUniformBuffer::mapMemory(uint64 p_size, uint64 p_offset) -> UBOMappedData
	{
		return m_device->mapMemory(m_bufferMemory, p_offset, p_size, {});
	}

	auto VKUniformBuffer::unmapMemory() -> void
	{
		m_device->unmapMemory(m_bufferMemory);
	}

	VKUniformBufferPFF::VKUniformBufferPFF(VKLogicalDevice *p_device, uint64 p_size, uint32 p_frames_in_flight) : m_device(p_device),
																												  m_framesInFlightCount(p_frames_in_flight)
	{
		TST_ASSERT_MSG(p_device, "Device cannot be null");
		TST_ASSERT_MSG(p_frames_in_flight > 0, "Frames in flight cannot be 0");

		for (uint32 i{0u}; i < m_framesInFlightCount; ++i)
		{
			vk::Buffer &      buffer{m_uniformBuffers.emplace_back(nullptr)};
			vk::DeviceMemory &memory{m_uniformBufferMemories.emplace_back(nullptr)};
			m_device->createBuffer(p_size, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
								   buffer, memory);

			auto &descriptor_info{m_descriptorBufferInfos.emplace_back()};
			descriptor_info.buffer = buffer;
			descriptor_info.offset = 0;
			descriptor_info.range  = p_size;
		}
	}

	VKUniformBufferPFF::~VKUniformBufferPFF()
	{
		for (auto &buffer: m_uniformBuffers)
		{
			m_device->deferDestruction([device = m_device, b = buffer]() mutable-> void
			{
				device->destroyObject(b);
			});
		}

		for (auto &memory: m_uniformBufferMemories)
		{
			m_device->deferDestruction([device = m_device, m = memory]() mutable-> void
			{
				device->destroyObject(m);
			});
		}
	}

	auto VKUniformBufferPFF::populateWriteDescriptor(vk::WriteDescriptorSet &p_write_descriptor, uint32 p_frame_index) -> void
	{
		p_write_descriptor.pBufferInfo = &m_descriptorBufferInfos[p_frame_index];
		if (!p_write_descriptor.pBufferInfo->buffer)
			TST_ASSERT_MSG(false, "Oh no");
	}

	auto VKUniformBufferPFF::getDescriptorResourceHandle(uint32 p_frame_index) -> void *
	{
		return m_descriptorBufferInfos[p_frame_index].buffer;
	}

	auto VKUniformBufferPFF::getBuffer(uint32 p_frame_index) -> vk::Buffer &
	{
		return m_uniformBuffers.at(p_frame_index);
	}

	auto VKUniformBufferPFF::getBufferMemory(uint32 p_frame_index) -> vk::DeviceMemory &
	{
		return m_uniformBufferMemories.at(p_frame_index);
	}

	auto VKUniformBufferPFF::getDescriptorInfo(uint32 p_frame_index) const -> const vk::DescriptorBufferInfo &
	{
		return m_descriptorBufferInfos.at(p_frame_index);
	}

	auto VKUniformBufferPFF::getFramesInFlightCount() const -> uint32
	{
		return m_framesInFlightCount;
	}

	auto VKUniformBufferPFF::mapMemory(uint32 p_frame_index, uint64 p_size, uint64 p_offset) -> void *
	{
		return m_device->mapMemory(m_uniformBufferMemories.at(p_frame_index), p_offset, p_size, {});
	}

	auto VKUniformBufferPFF::unmapMemory(uint32 p_frame_index) -> void
	{
		m_device->unmapMemory(m_uniformBufferMemories.at(p_frame_index));
	}

	auto VKUniformBufferPFF::mapAllMemory(uint64 p_size, uint64 p_offset) -> UBOMappedDataPFF
	{
		UBOMappedDataPFF mapped{};
		for (auto &memory: m_uniformBufferMemories)
			mapped.emplace_back(m_device->mapMemory(memory, p_offset, p_size, {}));
		return mapped;
	}

	auto VKUniformBufferPFF::unmapAllMemory() -> void
	{
		for (auto &memory: m_uniformBufferMemories)
			m_device->unmapMemory(memory);
	}
}
