#pragma once

#include "../toast_gpu.hpp"
#include <vulkan/vulkan_raii.hpp>

namespace toaster::gpu
{
	using ImageExtent = vk::Extent3D;

	struct TST_GPU_API ImageLayoutInfo
	{
		vk::ImageLayout         layout{vk::ImageLayout::eUndefined};
		vk::AccessFlags2        accessMask{vk::AccessFlagBits2::eNone};
		vk::PipelineStageFlags2 stageMask{vk::PipelineStageFlagBits2::eNone};
	};

	namespace util
	{
		constexpr auto hasStencilComponent(vk::Format p_format) -> bool
		{
			return p_format == vk::Format::eD32SfloatS8Uint || p_format == vk::Format::eD24UnormS8Uint;
		}

		constexpr auto isDepthFormat(vk::Format p_format) -> bool
		{
			return p_format == vk::Format::eD16Unorm || p_format == vk::Format::eD16UnormS8Uint || p_format == vk::Format::eD24UnormS8Uint || p_format ==
				   vk::Format::eD32Sfloat || p_format == vk::Format::eD32SfloatS8Uint;
		}

		constexpr auto getImageAspectMask(vk::Format p_format) -> vk::ImageAspectFlags
		{
			vk::ImageAspectFlags aspect_mask{isDepthFormat(p_format) ? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor};
			aspect_mask |= hasStencilComponent(p_format) ? vk::ImageAspectFlagBits::eStencil : vk::ImageAspectFlagBits::eNone;
			return aspect_mask;
		}

		constexpr auto getImageUsageFlags(vk::Format p_format, vk::SampleCountFlagBits p_sample_count) -> vk::ImageUsageFlags
		{
			vk::ImageUsageFlags usage_flags{};
			usage_flags |= isDepthFormat(p_format) ? vk::ImageUsageFlagBits::eDepthStencilAttachment : vk::ImageUsageFlagBits::eColorAttachment;
			usage_flags |= (p_sample_count != vk::SampleCountFlagBits::e1) ? vk::ImageUsageFlagBits::eTransientAttachment : vk::ImageUsageFlagBits{0};
			return usage_flags;
		}

		constexpr auto getBytesPerPixel([[maybe_unused]] vk::Format p_format) -> uint64
		{
			return 0u;
		}

		TST_GPU_API constexpr auto getImageLayoutInfo(vk::ImageLayout p_layout) -> const ImageLayoutInfo &;
	}
}
