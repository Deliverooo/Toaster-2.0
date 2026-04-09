#pragma once

#include <vulkan/vulkan_raii.hpp>
#include "toast_lib/core_basic.hpp"

namespace toaster::gpu
{
	class VKGPUContext;

	struct RenderingAttachmentInfo
	{
		vk::ImageView           imageView{};
		vk::ImageLayout         imageLayout{vk::ImageLayout::eUndefined};
		vk::ResolveModeFlagBits resolveMode{vk::ResolveModeFlagBits::eNone};
		vk::ImageView           resolveImageView{};
		vk::ImageLayout         resolveImageLayout{vk::ImageLayout::eUndefined};
		vk::AttachmentLoadOp    loadOp{vk::AttachmentLoadOp::eLoad};
		vk::AttachmentStoreOp   storeOp{vk::AttachmentStoreOp::eStore};
		vk::ClearValue          clearValue{};
	};

	struct RenderingInfo
	{
		vk::RenderingFlags flags{};
		vk::Rect2D         renderArea{};
		uint32             layerCount{1u};

		std::vector<RenderingAttachmentInfo> colourAttachments;
		RenderingAttachmentInfo *            pDepthAttachment{nullptr};
		RenderingAttachmentInfo *            pStencilAttachment{nullptr};
	};
}
