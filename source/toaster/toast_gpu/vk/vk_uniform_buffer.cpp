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

	VKUniformBufferPFF::VKUniformBufferPFF(VKLogicalDevice *p_device, uint64 p_size, uint32 p_frames_in_flight) : m_device(p_device),
																												  m_framesInFlightCount(p_frames_in_flight)
	{
		TST_ASSERT_MSG(p_device, "Device cannot be null");
		TST_ASSERT_MSG(p_frames_in_flight > 0, "Frames in flight cannot be 0");

		for (uint32 i{0u}; i < m_framesInFlightCount; ++i)
		{
			vk::raii::Buffer       &buffer{m_uniformBuffers.emplace_back(nullptr)};
			vk::raii::DeviceMemory &memory{m_uniformBufferMemories.emplace_back(nullptr)};
			m_device->createBuffer(p_size, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
								   buffer, memory);

			auto &descriptor_info{m_descriptorBufferInfos.emplace_back()};
			descriptor_info.buffer = buffer;
			descriptor_info.offset = 0;
			descriptor_info.range  = p_size;
		}
	}

	auto VKUniformBufferPFF::operator=(VKUniformBufferPFF &&p_other) noexcept -> VKUniformBufferPFF &
	{
		if (this != &p_other)
		{
			m_device                = p_other.m_device;
			m_uniformBuffers        = std::move(p_other.m_uniformBuffers);
			m_uniformBufferMemories = std::move(p_other.m_uniformBufferMemories);
			m_descriptorBufferInfos = p_other.m_descriptorBufferInfos;
			m_framesInFlightCount   = p_other.m_framesInFlightCount;
		}

		return *this;
	}

	auto VKUniformBufferPFF::getBuffer(uint32 p_frame_index) -> vk::raii::Buffer &
	{
		return m_uniformBuffers.at(p_frame_index);
	}

	auto VKUniformBufferPFF::getBufferMemory(uint32 p_frame_index) -> vk::raii::DeviceMemory &
	{
		return m_uniformBufferMemories.at(p_frame_index);
	}

	auto VKUniformBufferPFF::getDescriptorInfo(uint32 p_frame_index) const -> const vk::DescriptorBufferInfo &
	{
		return m_descriptorBufferInfos.at(p_frame_index);
	}

	auto VKUniformBufferPFF::mapMemory(uint32 p_frame_index, uint64 p_size, uint64 p_offset) -> void *
	{
		return m_uniformBufferMemories.at(p_frame_index).mapMemory(p_offset, p_size);
	}

	auto VKUniformBufferPFF::unmapMemory(uint32 p_frame_index) -> void
	{
		m_uniformBufferMemories.at(p_frame_index).unmapMemory();
	}

	auto VKUniformBufferPFF::mapAllMemory(uint64 p_size, uint64 p_offset) -> std::vector<void *>
	{
		std::vector<void *> mapped{};
		for (auto &memory: m_uniformBufferMemories)
			mapped.emplace_back(memory.mapMemory(p_offset, p_size));
		return mapped;
	}

	auto VKUniformBufferPFF::unmapAllMemory() -> void
	{
		for (auto &memory: m_uniformBufferMemories)
			memory.unmapMemory();
	}
}
