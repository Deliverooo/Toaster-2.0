#pragma once

#include "vk_buffer.hpp"

namespace toaster::gpu
{
	class TST_GPU_API VKIndexBuffer : public VKBuffer
	{
	public:
		VKIndexBuffer(VKGPUContext &p_gpu_ctx, uint64 p_size, bool32 p_device_local = false);

		auto getDeviceAddress() const -> uintptr;

	private:
		uintptr        m_deviceAddress{0u};
	};

	TST_GPU_DEFINE_HANDLE(VKIndexBuffer, IndexBuffer)
}
