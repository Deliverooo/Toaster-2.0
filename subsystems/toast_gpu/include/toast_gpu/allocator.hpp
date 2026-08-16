#pragma once

#include "logical_device.hpp"

#define VMA_CALL_PRE TST_GPU_API

#include <vma/vk_mem_alloc.h>

namespace toaster::gpu
{
	class TST_GPU_API Allocator
	{
	public:
		Allocator(Instance &p_instance, PhysicalDevice &p_physical_device, LogicalDevice &p_logical_device);
		~Allocator();

		auto getAllocator() const -> VmaAllocator { return m_allocator; }

	private:
		VmaAllocator m_allocator{nullptr};
	};
}
