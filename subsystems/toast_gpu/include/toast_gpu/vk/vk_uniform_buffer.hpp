#pragma once

#include "vk_buffer.hpp"
#include "vk_descriptor_heap.hpp"

namespace toaster::gpu
{
	class TST_GPU_API VKUniformBuffer : public VKBuffer
	{
	public:
		VKUniformBuffer(VKGPUContext &p_gpu_ctx, uint64 p_size, bool32 p_device_local = false);
		~VKUniformBuffer();

		auto getDeviceAddress() const -> uintptr;

		auto getDescriptorSlot() const -> DescriptorSlot;
		auto getHeapID() const -> uint32;

	private:
		uintptr        m_deviceAddress{0u};
		DescriptorSlot m_descriptorSlot{UINT32_MAX};
	};

	TST_GPU_DEFINE_HANDLE(VKUniformBuffer, UniformBuffer)
}
