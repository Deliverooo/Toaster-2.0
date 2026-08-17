#pragma once

#include "logical_device.hpp"

#define VMA_CALL_PRE TST_GPU_API

#include <vma/vk_mem_alloc.h>

namespace toaster::gpu
{
	// Very useful
	struct TST_GPU_API GPUBuffer
	{
		vk::Buffer    buffer{nullptr};
		VmaAllocation allocation{nullptr};
	};

	// I won't have an image view because descriptor heaps don't actually require them, only their vk::ImageViewCreateInfo.
	struct TST_GPU_API GPUImage
	{
		vk::Image     image{nullptr};
		VmaAllocation allocation{nullptr};
	};

	class TST_GPU_API Allocator
	{
	public:
		Allocator(Instance &p_instance, PhysicalDevice &p_physical_device, LogicalDevice &p_logical_device);
		~Allocator();

		auto getAllocator() const -> VmaAllocator { return m_allocator; }

		[[nodiscard]] auto createBuffer(uint64 p_size, vk::BufferUsageFlags p_usage_flags, VmaAllocationCreateFlags p_allocation_flags) -> GPUBuffer;
		auto               destroyBuffer(GPUBuffer &p_buffer) const -> void;

		[[nodiscard]] auto createImage(const vk::ImageCreateInfo &p_create_info, VmaAllocationCreateFlags p_allocation_flags) -> GPUImage;
		auto               destroyImage(GPUImage &p_image) const -> void;

	private:
		VmaAllocator m_allocator{nullptr};
	};
}
