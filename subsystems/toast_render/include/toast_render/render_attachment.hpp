#pragma once

#include "toast_render.hpp"

#include "toast_gpu/vk/vk_common.hpp"
#include "toast_gpu/vk/vk_raw_image.hpp"

namespace toaster::render
{
	// Just all the possible typical permutations of load and store ops
	enum class EAttachmentUsageOP
	{
		eClearStore,
		eLoadStore,
		eNoneStore,
		eDontCareStore,
		eClearNone,
		eLoadNone,
		eNoneNone,
		eDontCareNone,
		eClearDontCare,
		eLoadDontCare,
		eNoneDontCare,
		eDontCareDontCare
	};

	struct TST_RENDER_API RenderingAttachmentInfo
	{
		gpu::RawImage * image{nullptr};
		vk::ImageView   imageView{nullptr};
		vk::ImageLayout imageLayout{vk::ImageLayout::eUndefined};

		gpu::RawImage *         resolveImage{nullptr};
		vk::ResolveModeFlagBits resolveMode{vk::ResolveModeFlagBits::eNone};
		vk::ImageView           resolveImageView{nullptr};
		vk::ImageLayout         resolveImageLayout{vk::ImageLayout::eUndefined};

		EAttachmentUsageOP attachmentOp{EAttachmentUsageOP::eClearStore};
		vk::ClearValue     clearValue{};
	};

	struct TST_RENDER_API RenderingInfo
	{
		vk::Rect2D renderArea{};
		uint32     layerCount{1u};

		std::vector<RenderingAttachmentInfo> colourAttachments;

		std::optional<RenderingAttachmentInfo> depthAttachment{nullptr};
		bool                                   depthReadOnly{false};
		RenderingAttachmentInfo *              pStencilAttachment{nullptr};
		bool                                   stencilReadOnly{false};
	};

	TST_RENDER_API auto getLoadOp(EAttachmentUsageOP p_usage_op) -> vk::AttachmentLoadOp;
	TST_RENDER_API auto getStoreOp(EAttachmentUsageOP p_usage_op) -> vk::AttachmentStoreOp;

	[[nodiscard]] TST_RENDER_API auto getRenderingAttachmentInfo(gpu::RawImage &    p_image,
																 EAttachmentUsageOP p_usage_op = EAttachmentUsageOP::eClearStore) -> RenderingAttachmentInfo;

	[[nodiscard]] TST_RENDER_API auto getRenderingAttachmentInfo(gpu::RawImage &    p_image, gpu::RawImage &p_resolve_image,
																 EAttachmentUsageOP p_usage_op = EAttachmentUsageOP::eClearStore) -> RenderingAttachmentInfo;

	[[nodiscard]] TST_RENDER_API auto getRenderingArea(tsm::uint2 p_viewport_size, tsm::uint2 p_viewport_offset = tsm::uint2{0u}) -> vk::Rect2D;
}
