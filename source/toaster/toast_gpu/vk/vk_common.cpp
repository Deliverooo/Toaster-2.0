#include "vk_common.hpp"
#include <unordered_map>

#include "toast_lib/map.hpp"

namespace toaster::gpu::util
{
	constexpr ImageLayoutInfo c_colourAttachmentLayoutInfo{
		vk::ImageLayout::eColorAttachmentOptimal,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput
	};
	constexpr ImageLayoutInfo c_depthAttachmentLayoutInfo{
		vk::ImageLayout::eDepthAttachmentOptimal,
		vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
		vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests
	};
	constexpr ImageLayoutInfo c_depthAttachmentReadLayoutInfo{
		vk::ImageLayout::eDepthReadOnlyOptimal,
		vk::AccessFlagBits2::eDepthStencilAttachmentRead,
		vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests
	};
	constexpr ImageLayoutInfo c_shaderReadLayoutInfo{
		vk::ImageLayout::eShaderReadOnlyOptimal,
		vk::AccessFlagBits2::eShaderRead,
		vk::PipelineStageFlagBits2::eFragmentShader
	};
	constexpr ImageLayoutInfo c_transferSrcLayoutInfo{vk::ImageLayout::eTransferSrcOptimal, vk::AccessFlagBits2::eTransferRead, vk::PipelineStageFlagBits2::eTransfer};
	constexpr ImageLayoutInfo c_transferDstLayoutInfo{vk::ImageLayout::eTransferDstOptimal, vk::AccessFlagBits2::eTransferWrite, vk::PipelineStageFlagBits2::eTransfer};
	constexpr ImageLayoutInfo c_generalLayoutInfo{
		vk::ImageLayout::eGeneral,
		vk::AccessFlagBits2::eShaderRead | vk::AccessFlagBits2::eShaderWrite,
		vk::PipelineStageFlagBits2::eComputeShader | vk::PipelineStageFlagBits2::eFragmentShader
	};
	constexpr ImageLayoutInfo c_undefinedLayoutInfo{vk::ImageLayout::eUndefined, vk::AccessFlagBits2::eNone, vk::PipelineStageFlagBits2::eNone};

	constexpr MapConstexpr<vk::ImageLayout, ImageLayoutInfo, 8> c_imageLayoutInfoMap{
		{vk::ImageLayout::eColorAttachmentOptimal, c_colourAttachmentLayoutInfo},
		{vk::ImageLayout::eDepthAttachmentOptimal, c_depthAttachmentLayoutInfo},
		{vk::ImageLayout::eDepthReadOnlyOptimal, c_depthAttachmentReadLayoutInfo},
		{vk::ImageLayout::eShaderReadOnlyOptimal, c_shaderReadLayoutInfo},
		{vk::ImageLayout::eTransferSrcOptimal, c_transferSrcLayoutInfo},
		{vk::ImageLayout::eTransferDstOptimal, c_transferDstLayoutInfo},
		{vk::ImageLayout::eGeneral, c_generalLayoutInfo},
		{vk::ImageLayout::eUndefined, c_undefinedLayoutInfo}
	};

	constexpr auto getImageLayoutInfo(vk::ImageLayout p_layout) -> const ImageLayoutInfo &
	{
		return c_imageLayoutInfoMap.at(p_layout);
	}
}
