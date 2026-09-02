#pragma once

#include "descriptor_heap.hpp"
#include "toast_lib/pool.hpp"

namespace toaster::gpu
{
	struct TST_GPU_API TextureData
	{
		vk::Extent3D extent;

		vk::Image     image{nullptr};
		vk::ImageView imageView{nullptr}; // Completely optional unless you are creating a render target
		VmaAllocation allocation{nullptr};

		vk::ImageUsageFlags usageFlags{};
		uint32              layerCount{1u};
		uint32              mipLevels{1u};

		vk::Format      format{vk::Format::eUndefined};
		vk::ImageLayout layout{vk::ImageLayout::eUndefined};
		vk::ImageType   type{vk::ImageType::e2D};

		DescriptorSlot shaderReadHeapID{invalidImageDescriptorSlot};
		DescriptorSlot storageHeapID{invalidImageDescriptorSlot};
	};

	TST_DECLARE_HANDLE(Texture);

	struct TST_GPU_API TextureDesc
	{
		vk::Extent3D extent;

		vk::Image existingImage{nullptr}; // Use for swapchain images

		vk::ImageUsageFlags usageFlags{}; // Determines if the device should create image views for render attachments
		uint32              layerCount{1u};
		uint32              mipLevels{1u};

		vk::Format    format{vk::Format::eUndefined};
		vk::ImageType type{vk::ImageType::e2D};

		bool createDescriptors{false}; // Creates the heap id's depending on the image's usage flags
	};
}