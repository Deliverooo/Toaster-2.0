#include "vk_storage_buffer.hpp"
#include "vk_logical_device.hpp"

namespace toaster::gpu
{
	VKStorageBuffer::VKStorageBuffer(VKLogicalDevice *p_device, uint64 p_size) : m_device(p_device)
	{
		TST_ASSERT_MSG(p_device, "Context cannot be null");

		m_device->createBuffer(p_size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eStorageBuffer,
							   vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, m_buffer, m_bufferMemory);

		m_descriptorInfo.buffer = m_buffer;
		m_descriptorInfo.offset = 0;
		m_descriptorInfo.range  = p_size;

		TST_PERMA_ASSERT(m_buffer);
	}

	VKStorageBuffer::~VKStorageBuffer()
	{
		m_device->deferDestruction([device = m_device, buffer = m_buffer, buffer_memory = m_bufferMemory]() mutable-> void
		{
			device->destroyObject(buffer);
			device->destroyObject(buffer_memory);
		});
	}

	auto VKStorageBuffer::getBuffer() -> vk::Buffer &
	{
		return m_buffer;
	}

	auto VKStorageBuffer::getBufferMemory() -> vk::DeviceMemory &
	{
		return m_bufferMemory;
	}

	auto VKStorageBuffer::mapMemory(uint64 p_offset, uint64 p_size) -> void *
	{
		return m_device->mapMemory(m_bufferMemory, p_offset, p_size, {});
	}

	auto VKStorageBuffer::unmapMemory() -> void
	{
		m_device->unmapMemory(m_bufferMemory);
	}

	auto VKStorageBuffer::getDescriptorInfo() const -> const vk::DescriptorBufferInfo &
	{
		return m_descriptorInfo;
	}

	auto VKStorageBuffer::resize(uint64 p_size) -> void
	{
		m_buffer         = nullptr;
		m_descriptorInfo = vk::DescriptorBufferInfo{};

		m_device->createBuffer(p_size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eStorageBuffer,
							   vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, m_buffer, m_bufferMemory);

		m_descriptorInfo.buffer = m_buffer;
		m_descriptorInfo.offset = 0;
		m_descriptorInfo.range  = p_size;
	}

	VKStorageBufferPFF::VKStorageBufferPFF(VKLogicalDevice *p_device, uint64 p_size, uint32 p_frames_in_flight) : m_device(p_device),
																												  m_framesInFlightCount(p_frames_in_flight)
	{
		TST_ASSERT_MSG(p_device, "Device cannot be null");
		TST_ASSERT_MSG(p_frames_in_flight > 0, "Frames in flight cannot be 0");

		for (uint32 i{0u}; i < m_framesInFlightCount; ++i)
		{
			vk::Buffer &      buffer{m_storageBuffers.emplace_back(nullptr)};
			vk::DeviceMemory &memory{m_storageBufferMemories.emplace_back(nullptr)};
			m_device->createBuffer(p_size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eStorageBuffer,
								   vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, buffer, memory);

			auto &descriptor_info{m_descriptorBufferInfos.emplace_back()};
			descriptor_info.buffer = buffer;
			descriptor_info.offset = 0;
			descriptor_info.range  = p_size;
		}
	}

	VKStorageBufferPFF::~VKStorageBufferPFF()
	{
		for (auto &buffer: m_storageBuffers)
		{
			m_device->deferDestruction([device = m_device, b = buffer]() mutable-> void
			{
				device->destroyObject(b);
			});
		}

		for (auto &memory: m_storageBufferMemories)
		{
			m_device->deferDestruction([device = m_device, m = memory]() mutable-> void
			{
				device->destroyObject(m);
			});
		}
	}

	auto VKStorageBufferPFF::getBuffer(uint32 p_frame_index) -> vk::Buffer &
	{
		return m_storageBuffers.at(p_frame_index);
	}

	auto VKStorageBufferPFF::getBufferMemory(uint32 p_frame_index) -> vk::DeviceMemory &
	{
		return m_storageBufferMemories.at(p_frame_index);
	}

	auto VKStorageBufferPFF::getDescriptorInfo(uint32 p_frame_index) const -> const vk::DescriptorBufferInfo &
	{
		return m_descriptorBufferInfos.at(p_frame_index);
	}

	auto VKStorageBufferPFF::mapMemory(uint32 p_frame_index, uint64 p_size, uint64 p_offset) -> void *
	{
		return m_device->mapMemory(m_storageBufferMemories.at(p_frame_index), p_offset, p_size, {});
	}

	auto VKStorageBufferPFF::unmapMemory(uint32 p_frame_index) -> void
	{
		m_device->unmapMemory(m_storageBufferMemories.at(p_frame_index));
	}

	auto VKStorageBufferPFF::mapAllMemory(uint64 p_size, uint64 p_offset) -> std::vector<void *>
	{
		std::vector<void *> mapped{};
		for (auto &memory: m_storageBufferMemories)
			mapped.emplace_back(m_device->mapMemory(memory, p_offset, p_size, {}));
		return mapped;
	}

	auto VKStorageBufferPFF::unmapAllMemory() -> void
	{
		for (auto &memory: m_storageBufferMemories)
			m_device->unmapMemory(memory);
	}
}
