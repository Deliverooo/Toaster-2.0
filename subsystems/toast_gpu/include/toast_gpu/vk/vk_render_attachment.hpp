#pragma once

#include <vulkan/vulkan_raii.hpp>
#include "vk_texture.hpp"

namespace toaster::gpu
{
	struct RenderingAttachmentInfo
	{
		VKRawImage *image{nullptr};

		vk::ImageView   imageView{nullptr};
		vk::ImageLayout imageLayout{vk::ImageLayout::eUndefined};

		VKRawImage *            resolveImage{nullptr};
		vk::ResolveModeFlagBits resolveMode{vk::ResolveModeFlagBits::eNone};
		vk::ImageView           resolveImageView{nullptr};
		vk::ImageLayout         resolveImageLayout{vk::ImageLayout::eUndefined};

		vk::AttachmentLoadOp  loadOp{vk::AttachmentLoadOp::eClear};
		vk::AttachmentStoreOp storeOp{vk::AttachmentStoreOp::eStore};
		vk::ClearValue        clearValue{};
	};

	struct RenderingInfo
	{
		vk::RenderingFlags flags{};
		vk::Rect2D         renderArea{};
		uint32             layerCount{1u};

		std::vector<RenderingAttachmentInfo> colourAttachments;

		std::optional<RenderingAttachmentInfo> depthAttachment{nullptr};
		bool                                   depthReadOnly{false};
		RenderingAttachmentInfo *              pStencilAttachment{nullptr};
		bool                                   stencilReadOnly{false};
	};

	#pragma region attachment operations
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

	constexpr auto getLoadOp(EAttachmentUsageOP p_usage_op) -> vk::AttachmentLoadOp
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

	constexpr auto getStoreOp(EAttachmentUsageOP p_usage_op) -> vk::AttachmentStoreOp
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
	#pragma endregion
}
