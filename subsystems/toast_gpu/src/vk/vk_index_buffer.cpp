#include "toast_gpu/vk/vk_index_buffer.hpp"

#include "toast_gpu/vk/vk_gpu_context.hpp"

namespace toaster::gpu
{
	VKIndexBuffer::VKIndexBuffer(VKGPUContext &p_gpu_ctx, uint64 p_size, bool32 p_device_local) : VKBuffer(p_gpu_ctx, p_size, BufferSpecInfo{
																											   vk::BufferUsageFlagBits2::eIndexBuffer | (
																												   (p_device_local)
																													   ? vk::BufferUsageFlagBits2::eTransferDst
																													   : vk::BufferUsageFlagBits2{0u}),
																											   vk::QueueFlagBits::eGraphics,
																											   p_device_local
																										   })
	{
		m_deviceAddress = VKBuffer::getDeviceAddress();
	}

	auto VKIndexBuffer::getDeviceAddress() const -> uintptr
	{
		return m_deviceAddress;
	}
}
