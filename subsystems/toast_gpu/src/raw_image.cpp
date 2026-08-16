#include "toast_gpu/raw_image.hpp"

namespace toaster::gpu
{
	auto createRawImage(const Allocator &           p_allocator, RawImage &p_out_image, const vk::ImageCreateInfo &p_create_info,
						VmaAllocationCreateFlagBits p_allocation_flags) -> void
	{
		p_out_image.width         = p_create_info.extent.width;
		p_out_image.height        = p_create_info.extent.height;
		p_out_image.format        = p_create_info.format;
		p_out_image.currentLayout = p_create_info.initialLayout;

		VmaAllocationCreateInfo image_allocation_create_info{};
		image_allocation_create_info.usage = VMA_MEMORY_USAGE_AUTO;
		image_allocation_create_info.flags = p_allocation_flags;
		vmaCreateImage(p_allocator.getAllocator(), reinterpret_cast<const VkImageCreateInfo *>(&p_create_info), &image_allocation_create_info,
					   reinterpret_cast<VkImage *>(&p_out_image.image), &p_out_image.allocation, nullptr);
	}

	auto destroyRawImage(const Allocator &p_allocator, RawImage &p_image) -> void
	{
		vmaDestroyImage(p_allocator.getAllocator(), p_image.image, p_image.allocation);
		p_image.image      = nullptr;
		p_image.allocation = nullptr;
	}
}
