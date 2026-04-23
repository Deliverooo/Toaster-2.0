#pragma once

#include <vulkan/vulkan_raii.hpp>
#include "vk_texture.hpp"

namespace toaster::gpu
{
	struct RenderingAttachmentInfo
	{
		RefPtr<VKImage2D> image{nullptr};

		vk::ImageView   imageView{nullptr};
		vk::ImageLayout imageLayout{vk::ImageLayout::eUndefined};

		RefPtr<VKImage2D>       resolveImage{nullptr};
		vk::ResolveModeFlagBits resolveMode{vk::ResolveModeFlagBits::eNone};
		vk::ImageView           resolveImageView{nullptr};
		vk::ImageLayout         resolveImageLayout{vk::ImageLayout::eUndefined};

		vk::AttachmentLoadOp  loadOp{vk::AttachmentLoadOp::eLoad};
		vk::AttachmentStoreOp storeOp{vk::AttachmentStoreOp::eStore};
		vk::ClearValue        clearValue{};
	};

	struct RenderingInfo
	{
		vk::RenderingFlags flags{};
		vk::Rect2D         renderArea{};
		uint32             layerCount{1u};

		std::vector<RenderingAttachmentInfo> colourAttachments;

		RenderingAttachmentInfo *pDepthAttachment{nullptr};
		bool                     depthReadOnly{false};
		RenderingAttachmentInfo *pStencilAttachment{nullptr};
		bool                     stencilReadOnly{false};
	};
}
