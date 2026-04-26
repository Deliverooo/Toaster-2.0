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
	}

	auto VKStorageBuffer::getBuffer() -> vk::raii::Buffer &
	{
		return m_buffer;
	}

	auto VKStorageBuffer::getBufferMemory() -> vk::raii::DeviceMemory &
	{
		return m_bufferMemory;
	}

	auto VKStorageBuffer::mapMemory(uint64 p_offset, uint64 p_size) -> void *
	{
		return m_bufferMemory.mapMemory(p_offset, p_size, {});
	}

	auto VKStorageBuffer::unmapMemory() -> void
	{
		m_bufferMemory.unmapMemory();
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

	auto VKStorageBuffer::getResourceType() const -> EGPUResourceType
	{
		return EGPUResourceType::eStorageBuffer;
	}

	VKStorageBufferPFF::VKStorageBufferPFF(VKLogicalDevice *p_device, uint64 p_size, uint32 p_frames_in_flight) : m_framesInFlightCount(p_frames_in_flight)
	{
		TST_ASSERT_MSG(p_device, "Context cannot be null");
		TST_ASSERT_MSG(p_frames_in_flight > 0, "Frames in flight cannot be 0");

		for (uint32 i{0u}; i < m_framesInFlightCount; ++i)
			m_storageBuffers.emplace_back(p_device->alloc<VKStorageBuffer>(p_size));
	}

	auto VKStorageBufferPFF::getSSBO(uint32 p_frame_index) -> RefPtr<VKStorageBuffer>
	{
		return m_storageBuffers.at(p_frame_index);
	}

	auto VKStorageBufferPFF::setSSBO(uint32 p_frame_index, const RefPtr<VKStorageBuffer> &p_storage_buffer) -> void
	{
		m_storageBuffers.at(p_frame_index) = p_storage_buffer;
	}

	auto VKStorageBufferPFF::begin() -> std::vector<RefPtr<VKStorageBuffer> >::iterator
	{
		return m_storageBuffers.begin();
	}

	auto VKStorageBufferPFF::end() -> std::vector<RefPtr<VKStorageBuffer> >::iterator
	{
		return m_storageBuffers.end();
	}

	auto VKStorageBufferPFF::resize(uint64 p_size) -> void
	{
		for (auto &buffer: m_storageBuffers)
			buffer->resize(p_size);
	}

	auto VKStorageBufferPFF::getResourceType() const -> EGPUResourceType
	{
		return EGPUResourceType::eStorageBufferPFF;
	}
}
