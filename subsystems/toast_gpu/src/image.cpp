#include "toast_gpu/image.hpp"

namespace toaster::gpu
{
	auto createImage(const Allocator &           p_allocator, ResourceDescriptorHeap &p_resource_heap, Image &p_out_image, const vk::ImageCreateInfo &p_create_info,
					 VmaAllocationCreateFlagBits p_allocation_flags) -> void
	{
		createRawImage(p_allocator, p_out_image.rawImage, p_create_info, p_allocation_flags);
	}

	auto destroyImage(const Allocator &p_allocator, ResourceDescriptorHeap &p_resource_heap, Image &p_image) -> void
	{
		destroyRawImage(p_allocator, p_image.rawImage);

		if (p_image.shaderReadHeapID != UINT32_MAX)
			p_resource_heap.freeImageSlot(p_image.shaderReadHeapID);
		if (p_image.storageHeapID != UINT32_MAX)
			p_resource_heap.freeImageSlot(p_image.storageHeapID);

		p_image.shaderReadHeapID = UINT32_MAX;
		p_image.storageHeapID    = UINT32_MAX;
	}
}
