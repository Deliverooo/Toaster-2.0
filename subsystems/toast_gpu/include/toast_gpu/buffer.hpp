#pragma once

#include "descriptor_heap.hpp"
#include "toast_lib/pool.hpp"

namespace toaster::gpu
{
	struct TST_GPU_API BufferData
	{
		vk::Buffer        buffer{nullptr};
		VmaAllocation     allocation{nullptr};
		vk::DeviceSize    size{0u};
		vk::DeviceAddress address{0u};

		void *mapped{nullptr}; // nullptr if the buffer is not host visible / coherent

		vk::BufferUsageFlags usageFlags{};
		EMemoryProperties    memoryProperties{EMemoryProperties::eDeviceLocal};
	};

	TST_DECLARE_HANDLE(Buffer);

	struct TST_GPU_API BufferDesc
	{
		static BufferDesc staging(vk::DeviceSize p_size)
		{
			return BufferDesc{p_size, vk::BufferUsageFlagBits::eTransferSrc, EMemoryProperties::eHostVisibleCoherent};
		}

		vk::DeviceSize       size{0u};
		vk::BufferUsageFlags usageFlags{};
		EMemoryProperties    memoryProperties{EMemoryProperties::eDeviceLocal};
	};
}
