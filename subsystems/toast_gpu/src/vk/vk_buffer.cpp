#include "toast_gpu/vk/vk_buffer.hpp"

#include "toast_gpu/vk/vk_gpu_context.hpp"

namespace toaster::gpu
{
	VKBuffer::VKBuffer(VKGPUContext &p_gpu_ctx, uint64 p_size, const BufferSpecInfo &p_spec_info) : m_gpuCtx(&p_gpu_ctx), m_specInfo(p_spec_info), m_size(p_size)
	{
		m_specInfo.usageFlags |= vk::BufferUsageFlagBits2::eShaderDeviceAddressKHR;

		if (!m_specInfo.deviceLocal)
		{
			m_gpuCtx->getLogicalDevice()->createBuffer(m_buffer, m_bufferMemory, p_size, m_specInfo.usageFlags,
													   vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, m_specInfo.queueAccessFlags);
		}
		else
		{
			m_gpuCtx->getLogicalDevice()->createBuffer(m_buffer, m_bufferMemory, p_size, vk::BufferUsageFlagBits2::eTransferDst | m_specInfo.usageFlags,
													   vk::MemoryPropertyFlagBits::eDeviceLocal, m_specInfo.queueAccessFlags);
		}
	}

	VKBuffer::~VKBuffer()
	{
		m_gpuCtx->deferDestruction([buffer = std::move(m_buffer), buffer_memory = std::move(m_bufferMemory)]() mutable -> void
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
		return m_gpuCtx->getLogicalDevice()->getVulkanLogicalDevice().getBufferAddressKHR({*m_buffer});
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

	auto VKBuffer::copyData(const void *p_data, uint64 p_size) -> void
	{
		TST_PERMA_ASSERT(!m_specInfo.deviceLocal);

		void *mapped{mapMemory(p_size)};
		std::memcpy(mapped, p_data, p_size);
		unmapMemory();
	}

	auto VKBuffer::copyFromBuffer(VKBuffer &p_other) -> void
	{
		m_gpuCtx->copyBuffer(p_other.m_buffer, m_buffer, m_size);
	}
}
