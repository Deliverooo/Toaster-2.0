#pragma once

#include "descriptor_heap.hpp"

namespace toaster::gpu
{
	struct TST_GPU_API RawImage
	{
		vk::Image     image{nullptr};
		VmaAllocation allocation{nullptr};

		vk::Format      format{vk::Format::eUndefined};
		vk::ImageLayout currentLayout{vk::ImageLayout::eUndefined};

		uint32 width{0u};
		uint32 height{0u};
	};

	TST_GPU_API auto createRawImage(const Allocator &           p_allocator, RawImage &p_out_image, const vk::ImageCreateInfo &p_create_info,
									VmaAllocationCreateFlagBits p_allocation_flags) -> void;
	TST_GPU_API auto destroyRawImage(const Allocator &p_allocator, RawImage &p_image) -> void;
}
