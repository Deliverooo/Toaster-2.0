#include "toast_render/render_attachment.hpp"

namespace toaster::render
{
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
