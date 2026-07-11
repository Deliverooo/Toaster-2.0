#pragma once

#include "vk_buffer.hpp"
#include "vk_descriptor_heap.hpp"

namespace toaster::gpu
{
	class TST_GPU_API VKStorageBuffer : public VKBuffer
	{
	public:
		VKStorageBuffer(VKGPUContext &p_gpu_ctx, uint64 p_size, bool32 p_device_local = false);
		~VKStorageBuffer();

		auto getDeviceAddress() const -> uintptr;

		auto getDescriptorSlot() const -> DescriptorSlot;
		auto getHeapID() const -> uint32;

	private:
		uintptr        m_deviceAddress{0u};
		DescriptorSlot m_descriptorSlot{UINT32_MAX};
	};

	TST_GPU_DEFINE_HANDLE(VKStorageBuffer, StorageBuffer)
	TST_GPU_DEFINE_HANDLE(VKStorageBuffer, VertexBuffer)
}
