#include "toast_gpu/vk/vk_storage_buffer.hpp"

#include "toast_gpu/vk/vk_gpu_context.hpp"

namespace toaster::gpu
{
	VKStorageBuffer::VKStorageBuffer(VKGPUContext &p_gpu_ctx, uint64 p_size, bool32 p_device_local) : VKBuffer(p_gpu_ctx, p_size, BufferSpecInfo{
																												   vk::BufferUsageFlagBits2::eStorageBuffer | (
																													   (p_device_local)
																														   ? vk::BufferUsageFlagBits2::eTransferDst
																														   : vk::BufferUsageFlagBits2{0u}),
																												   vk::QueueFlagBits::eGraphics,
																												   p_device_local
																											   })
	{
		m_descriptorSlot = m_gpuCtx->getDescriptorHeap()->allocBuffer(*this, true);
		m_deviceAddress  = VKBuffer::getDeviceAddress();
	}

	VKStorageBuffer::~VKStorageBuffer()
	{
		m_gpuCtx->getDescriptorHeap()->freeBuffer(m_descriptorSlot);
	}

	auto VKStorageBuffer::getDeviceAddress() const -> uintptr
	{
		return m_deviceAddress;
	}

	auto VKStorageBuffer::getDescriptorSlot() const -> DescriptorSlot
	{
		return m_descriptorSlot;
	}

	auto VKStorageBuffer::getHeapID() const -> uint32
	{
		return m_descriptorSlot + (m_gpuCtx->getDescriptorHeap()->getBufferOffset() / m_gpuCtx->getDescriptorHeap()->getBufferDescriptorSize());
	}
}
