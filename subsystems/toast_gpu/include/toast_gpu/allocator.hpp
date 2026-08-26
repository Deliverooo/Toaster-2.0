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

		[[nodiscard]] auto getAllocator() const -> VmaAllocator { return m_allocator; }

		auto createBuffer(uint64         p_size, vk::BufferUsageFlags p_usage_flags, VmaAllocationCreateFlags p_allocation_flags, vk::Buffer &p_out_buffer,
						  VmaAllocation &p_out_allocation, void **    p_out_mapped = nullptr) const -> void;
		auto destroyBuffer(vk::Buffer &p_buffer, VmaAllocation &p_allocation) const -> void;

		auto createImage(const vk::ImageCreateInfo &p_create_info, VmaAllocationCreateFlags p_allocation_flags, vk::Image &p_out_image,
						 VmaAllocation &            p_out_allocation) const -> void;
		auto destroyImage(vk::Image &p_image, VmaAllocation &p_allocation) const -> void;

	private:
		VmaAllocator m_allocator{nullptr};
	};
}
