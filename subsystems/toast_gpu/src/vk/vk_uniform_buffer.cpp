#include "toast_gpu/vk/vk_uniform_buffer.hpp"
#include "toast_gpu/vk/vk_gpu_context.hpp"

namespace toaster::gpu
{
	VKUniformBuffer::VKUniformBuffer(VKGPUContext &p_gpu_ctx, uint64 p_size, bool32 p_device_local) : VKBuffer(p_gpu_ctx, p_size, BufferSpecInfo{
																												   vk::BufferUsageFlagBits2::eUniformBuffer | (
																													   (p_device_local)
																														   ? vk::BufferUsageFlagBits2::eTransferDst
																														   : vk::BufferUsageFlagBits2{0u}),
																												   vk::QueueFlagBits::eGraphics,
																												   p_device_local
																											   })
	{
		m_descriptorSlot = m_gpuCtx->getDescriptorHeap()->allocBuffer(*this, false);
		m_deviceAddress  = VKBuffer::getDeviceAddress();
	}

	VKUniformBuffer::~VKUniformBuffer()
	{
		m_gpuCtx->getDescriptorHeap()->freeBuffer(m_descriptorSlot);
	}

	auto VKUniformBuffer::getDeviceAddress() const -> uintptr
	{
		return m_deviceAddress;
	}

	auto VKUniformBuffer::getDescriptorSlot() const -> DescriptorSlot
	{
		return m_descriptorSlot;
	}

	auto VKUniformBuffer::getHeapID() const -> uint32
	{
		return m_descriptorSlot + (m_gpuCtx->getDescriptorHeap()->getBufferOffset() / m_gpuCtx->getDescriptorHeap()->getBufferDescriptorSize());
	}
}
