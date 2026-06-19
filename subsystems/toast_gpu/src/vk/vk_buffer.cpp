#include "toast_gpu/vk/vk_buffer.hpp"

#include <ranges>

#include "toast_gpu/vk/vk_logical_device.hpp"

namespace toaster::gpu
{
	auto VKBuffer::operator=(VKBuffer &&p_other) noexcept -> VKBuffer &
	{
		if (this != &p_other)
		{
			m_device       = p_other.m_device;
			m_specInfo     = p_other.m_specInfo;
			m_size         = p_other.m_size;;
			m_buffer       = std::move(p_other.m_buffer);
			m_bufferMemory = std::move(p_other.m_bufferMemory);
		}
		return *this;
	}

	VKBuffer::VKBuffer(VKLogicalDevice *p_device, uint64 p_size, const BufferSpecInfo &p_spec_info) : m_device(p_device), m_specInfo(p_spec_info), m_size(p_size)
	{
		vk::BufferCreateInfo buffer_create_info{};
		buffer_create_info.size = m_size;

		const auto qfi{m_device->getQueueFamilyIndices(m_specInfo.queueAccessFlags)};
		buffer_create_info.queueFamilyIndexCount = qfi.size();
		auto qfi_vec{qfi | std::ranges::to<std::vector>()};
		buffer_create_info.pQueueFamilyIndices = qfi_vec.data();
		buffer_create_info.sharingMode         = (qfi.size() > 1 ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive);

		vk::BufferUsageFlags2CreateInfo buffer_flags_create_info{};
		buffer_flags_create_info.usage = m_specInfo.usageFlags;
		buffer_create_info.pNext       = &buffer_flags_create_info;

		m_buffer = {m_device->getVulkanLogicalDevice(), buffer_create_info};

		vk::MemoryRequirements memory_requirements = m_buffer.getMemoryRequirements();
		vk::MemoryAllocateInfo memory_allocate_info{};
		memory_allocate_info.memoryTypeIndex = m_device->getPhysicalDevice()->findMemoryType(memory_requirements.memoryTypeBits, m_specInfo.memoryPropertyFlags);
		memory_allocate_info.allocationSize  = memory_requirements.size;
		vk::MemoryAllocateFlagsInfo memory_alloc_flags_info{};
		memory_alloc_flags_info.flags = vk::MemoryAllocateFlagBits::eDeviceAddress;
		memory_allocate_info.pNext    = &memory_alloc_flags_info;

		m_bufferMemory = {m_device->getVulkanLogicalDevice(), memory_allocate_info};

		m_buffer.bindMemory(m_bufferMemory, 0u);
	}

	VKBuffer::~VKBuffer()
	{
		m_device->deferDestruction([buffer = std::move(m_buffer), buffer_memory = std::move(m_bufferMemory)]() mutable -> void
		{
		});
	}

	auto VKBuffer::getSpecInfo() const -> const BufferSpecInfo &
	{
		return m_specInfo;
	}

	auto VKBuffer::getSize() const -> vk::DeviceSize
	{
		return m_size;
	}

	auto VKBuffer::getBuffer() const -> vk::Buffer
	{
		return *m_buffer;
	}

	auto VKBuffer::getBufferMemory() const -> vk::DeviceMemory
	{
		return *m_bufferMemory;
	}

	auto VKBuffer::getDeviceAddress() const -> vk::DeviceAddress
	{
		return m_device->getVulkanLogicalDevice().getBufferAddressKHR({*m_buffer});
	}

	auto VKBuffer::getDeviceAddressRange() const -> vk::DeviceAddressRangeKHR
	{
		return vk::DeviceAddressRangeKHR{getDeviceAddress(), m_size};
	}

	auto VKBuffer::mapMemory(uint64 p_size, uint64 p_offset) -> void *
	{
		return m_bufferMemory.mapMemory(p_offset, p_size, {});
	}

	auto VKBuffer::unmapMemory() -> void
	{
		m_bufferMemory.unmapMemory();
	}

	auto VKBuffer::setData(const void *p_data, uint64 p_size) -> void
	{
		void *mapped{mapMemory(p_size)};
		std::memcpy(mapped, p_data, p_size);
		unmapMemory();
	}
}
