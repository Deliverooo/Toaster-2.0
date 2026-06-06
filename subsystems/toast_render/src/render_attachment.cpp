#include "toast_render/render_attachment.hpp"

namespace toaster::render
{
	auto RenderingAttachmentInfo::getVulkanAttachmentInfo() const -> vk::RenderingAttachmentInfo
	{
		vk::RenderingAttachmentInfo attachment_info{};

		if (image != nullptr)
		{
			attachment_info.imageView   = image->getImageView();
			attachment_info.imageLayout = image->getCurrentImageLayout();
		}
		else
		{
			attachment_info.imageView   = imageView;
			attachment_info.imageLayout = imageLayout;
		}

		if (resolveImage != nullptr)
		{
			attachment_info.resolveImageView   = resolveImage->getImageView();
			attachment_info.resolveImageLayout = resolveImage->getCurrentImageLayout();
		}
		else
		{
			attachment_info.resolveImageView   = resolveImageView;
			attachment_info.resolveImageLayout = resolveImageLayout;
		}

		attachment_info.resolveMode = resolveMode;

		attachment_info.loadOp     = getLoadOp(attachmentOp);
		attachment_info.storeOp    = getStoreOp(attachmentOp);
		attachment_info.clearValue = clearValue;

		return attachment_info;
	}

	auto RenderingInfo::getViewport() const -> vk::Viewport
	{
		const vk::Extent2D rendering_extent{renderArea.extent};
		const vk::Offset2D rendering_offset{renderArea.offset};

		const vk::Viewport viewport{
			static_cast<float32>(rendering_offset.x),
			static_cast<float32>(rendering_offset.y),
			static_cast<float32>(rendering_extent.width),
			static_cast<float32>(rendering_extent.height),
			0.0f,
			1.0f
		};
		return viewport;
	}

	auto RenderingInfo::getScissor() const -> vk::Rect2D
	{
		const vk::Extent2D rendering_extent{renderArea.extent};
		const vk::Offset2D rendering_offset{renderArea.offset};

		const vk::Rect2D scissor{rendering_offset, rendering_extent};
		return scissor;
	}

	auto RenderingInfo::getVulkanRenderingInfo() const -> vk::RenderingInfo
	{
		std::vector<vk::RenderingAttachmentInfo> colour_rendering_attachment_infos{};
		for (const auto &rendering_attachment: colourAttachments)
		{
			auto &info{colour_rendering_attachment_infos.emplace_back()};
			info = rendering_attachment.getVulkanAttachmentInfo();
		}

		vk::RenderingAttachmentInfo depth_attachment_info{};
		if (depthAttachment.has_value())
			depth_attachment_info = depthAttachment->getVulkanAttachmentInfo();

		vk::RenderingAttachmentInfo stencil_attachment_info{};
		if (pStencilAttachment != nullptr)
			stencil_attachment_info = pStencilAttachment->getVulkanAttachmentInfo();

		vk::RenderingInfo rendering_info{};
		rendering_info.renderArea           = renderArea;
		rendering_info.layerCount           = layerCount;
		rendering_info.colorAttachmentCount = colourAttachments.size();
		rendering_info.pColorAttachments    = colour_rendering_attachment_infos.empty() ? nullptr : colour_rendering_attachment_infos.data();
		rendering_info.pDepthAttachment     = depthAttachment.has_value() ? &depth_attachment_info : nullptr;
		rendering_info.pStencilAttachment   = pStencilAttachment ? &stencil_attachment_info : nullptr;

		return rendering_info;
	}

	auto getLoadOp(EAttachmentUsageOP p_usage_op) -> vk::AttachmentLoadOp
	{
		switch (p_usage_op)
		{
			case EAttachmentUsageOP::eClearStore:
			case EAttachmentUsageOP::eClearNone:
			case EAttachmentUsageOP::eClearDontCare:
				return vk::AttachmentLoadOp::eClear;
			case EAttachmentUsageOP::eLoadStore:
			case EAttachmentUsageOP::eLoadNone:
			case EAttachmentUsageOP::eLoadDontCare:
				return vk::AttachmentLoadOp::eLoad;
			case EAttachmentUsageOP::eNoneStore:
			case EAttachmentUsageOP::eNoneNone:
			case EAttachmentUsageOP::eNoneDontCare:
				return vk::AttachmentLoadOp::eNone;
			case EAttachmentUsageOP::eDontCareStore:
			case EAttachmentUsageOP::eDontCareNone:
			case EAttachmentUsageOP::eDontCareDontCare:
				return vk::AttachmentLoadOp::eDontCare;
		}
		return vk::AttachmentLoadOp::eNone;
	}

	auto getStoreOp(EAttachmentUsageOP p_usage_op) -> vk::AttachmentStoreOp
	{
		switch (p_usage_op)
		{
			case EAttachmentUsageOP::eClearStore:
			case EAttachmentUsageOP::eLoadStore:
			case EAttachmentUsageOP::eNoneStore:
			case EAttachmentUsageOP::eDontCareStore:
				return vk::AttachmentStoreOp::eStore;
			case EAttachmentUsageOP::eClearNone:
			case EAttachmentUsageOP::eLoadNone:
			case EAttachmentUsageOP::eNoneNone:
			case EAttachmentUsageOP::eDontCareNone:
				return vk::AttachmentStoreOp::eNone;
			case EAttachmentUsageOP::eClearDontCare:
			case EAttachmentUsageOP::eLoadDontCare:
			case EAttachmentUsageOP::eNoneDontCare:
			case EAttachmentUsageOP::eDontCareDontCare:
				return vk::AttachmentStoreOp::eDontCare;
		}
		return vk::AttachmentStoreOp::eNone;
	}

	auto getRenderingAttachmentInfo(gpu::RawImage &p_image, EAttachmentUsageOP p_usage_op) -> RenderingAttachmentInfo
	{
		RenderingAttachmentInfo attachment_info{};
		attachment_info.image        = &p_image;
		attachment_info.attachmentOp = p_usage_op;

		const vk::ImageAspectFlags aspect_mask{gpu::util::getImageAspectMask(p_image.getSpecInfo().format)};
		attachment_info.clearValue = gpu::util::getDefaultImageClearValue(aspect_mask);
		return attachment_info;
	}

	auto getRenderingAttachmentInfo(gpu::RawImage &p_image, gpu::RawImage &p_resolve_image, EAttachmentUsageOP p_usage_op) -> RenderingAttachmentInfo
	{
		RenderingAttachmentInfo attachment_info{};
		attachment_info.image        = &p_image;
		attachment_info.attachmentOp = p_usage_op;
		attachment_info.resolveImage = &p_resolve_image;

		const vk::ImageAspectFlags aspect_mask{gpu::util::getImageAspectMask(p_resolve_image.getSpecInfo().format)};
		attachment_info.resolveMode = gpu::util::getDefaultImageResolveMode(aspect_mask);
		attachment_info.clearValue  = gpu::util::getDefaultImageClearValue(aspect_mask);

		return attachment_info;
	}

	auto getRenderingArea(tsm::uint2 p_viewport_size, tsm::uint2 p_viewport_offset) -> vk::Rect2D
	{
		return vk::Rect2D{{static_cast<int32>(p_viewport_offset.x), static_cast<int32>(p_viewport_offset.y)}, {p_viewport_size.x, p_viewport_size.y}};
	}
}
