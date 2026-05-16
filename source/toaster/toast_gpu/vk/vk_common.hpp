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

		constexpr auto getBytesPerPixel([[maybe_unused]] vk::Format p_format) -> uint32
		{
			switch (p_format)
			{
				// 1 Byte (8 bits) per pixel
				case vk::Format::eR8Unorm:
				case vk::Format::eR8Snorm:
				case vk::Format::eR8Uint:
				case vk::Format::eR8Sint:
				case vk::Format::eR8Srgb:
				case vk::Format::eS8Uint:
					return 1u;

				// 2 Bytes (16 bits) per pixel
				case vk::Format::eR8G8Unorm:
				case vk::Format::eR8G8Snorm:
				case vk::Format::eR8G8Uint:
				case vk::Format::eR8G8Sint:
				case vk::Format::eR8G8Srgb:
				case vk::Format::eR16Unorm:
				case vk::Format::eR16Snorm:
				case vk::Format::eR16Uint:
				case vk::Format::eR16Sint:
				case vk::Format::eR16Sfloat:
				case vk::Format::eD16Unorm:
				case vk::Format::eR5G6B5UnormPack16:
				case vk::Format::eB5G6R5UnormPack16:
					return 2;

				// 3 Bytes (24 bits) per pixel
				case vk::Format::eR8G8B8Unorm:
				case vk::Format::eR8G8B8Snorm:
				case vk::Format::eR8G8B8Uint:
				case vk::Format::eR8G8B8Sint:
				case vk::Format::eR8G8B8Srgb:
				case vk::Format::eB8G8R8Unorm:
				case vk::Format::eB8G8R8Srgb:
					return 3;

				// 4 Bytes (32 bits) per pixel
				case vk::Format::eR8G8B8A8Unorm:
				case vk::Format::eR8G8B8A8Snorm:
				case vk::Format::eR8G8B8A8Uint:
				case vk::Format::eR8G8B8A8Sint:
				case vk::Format::eR8G8B8A8Srgb:
				case vk::Format::eB8G8R8A8Unorm:
				case vk::Format::eB8G8R8A8Srgb:
				case vk::Format::eR16G16Unorm:
				case vk::Format::eR16G16Sfloat:
				case vk::Format::eR32Uint:
				case vk::Format::eR32Sint:
				case vk::Format::eR32Sfloat:
				case vk::Format::eD32Sfloat:
				case vk::Format::eD24UnormS8Uint: // 24-bit depth, 8-bit stencil packed into 32 bits
				case vk::Format::eA2B10G10R10UnormPack32:
					return 4;

				// 6 Bytes (48 bits) per pixel
				case vk::Format::eR16G16B16Sfloat:
					return 6;

				// 8 Bytes (64 bits) per pixel
				case vk::Format::eR16G16B16A16Unorm:
				case vk::Format::eR16G16B16A16Sfloat:
				case vk::Format::eR32G32Sfloat:
				case vk::Format::eD32SfloatS8Uint: // 32-bit depth, 8-bit stencil + padding (64 bits total)
					return 8;

				// 12 Bytes (96 bits) per pixel
				case vk::Format::eR32G32B32Sfloat:
					return 12;

				// 16 Bytes (128 bits) per pixel
				case vk::Format::eR32G32B32A32Sfloat:
				case vk::Format::eR32G32B32A32Uint:
					return 16;

				// Block-compressed or undefined formats require special calculations
				default:
					return 0;
			}
		}

		TST_GPU_API constexpr auto getImageLayoutInfo(vk::ImageLayout p_layout) -> const ImageLayoutInfo &;
	}
}
