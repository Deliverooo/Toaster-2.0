#pragma once

#include "raw_image.hpp"

namespace toaster::gpu
{
	struct TST_GPU_API Image
	{
		RawImage rawImage;

		// Depending on the usage, these may be 'null'
		DescriptorSlot shaderReadHeapID{UINT32_MAX};
		DescriptorSlot storageHeapID{UINT32_MAX};
	};

	TST_GPU_API auto createImage(const Allocator &p_allocator, ResourceDescriptorHeap &p_resource_heap, Image &p_out_image, const vk::ImageCreateInfo &p_create_info,
								 VmaAllocationCreateFlagBits p_allocation_flags) -> void;
	TST_GPU_API auto destroyImage(const Allocator &p_allocator, ResourceDescriptorHeap &p_resource_heap, Image &p_image) -> void;
}
