#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace toaster::gpu
{
	enum class ERenderAttachmentType
	{
		eColour,
		eDepth,
		eStencil,
		eDepthStencil
	};

	struct VKRenderAttachment
	{
		ERenderAttachmentType type{ERenderAttachmentType::eColour};
		vk::ClearValue        clearValue{};

		vk::ImageView targetImage{nullptr};
	};
}
