#pragma once

#include <vulkan/vulkan.hpp>

#include "gpu_enums.hpp"

namespace toaster::gpu
{
	#define TST_REGISTER_DEPENDENCY(__class, __getname, __membername) private: ::toaster::NonOwningPtr<__class> m_##__membername{nullptr}; public: auto get##__getname () const -> __class* { return m_##__membername; } private:

	enum class EMemoryProperties : uint8
	{
		eDeviceLocal, eHostVisibleCoherent
	};

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

	constexpr auto isRenderTarget(vk::ImageUsageFlags p_usage_flags) -> bool
	{
		return (p_usage_flags & vk::ImageUsageFlagBits::eColorAttachment) || (p_usage_flags & vk::ImageUsageFlagBits::eDepthStencilAttachment);
	}

	constexpr auto getImageAccessFlagsAndStageMask(vk::ImageLayout p_layout, vk::AccessFlags2 &p_out_access_flags, vk::PipelineStageFlags2 &p_out_stage_mask) -> void
	{
		switch (p_layout)
		{
			case vk::ImageLayout::eUndefined:
			{
				p_out_access_flags = vk::AccessFlagBits2::eNone;
				p_out_stage_mask   = vk::PipelineStageFlagBits2::eNone;
			}
			case vk::ImageLayout::eGeneral:
			{
				p_out_access_flags = vk::AccessFlagBits2::eShaderRead | vk::AccessFlagBits2::eShaderWrite;
				p_out_stage_mask   = vk::PipelineStageFlagBits2::eAllCommands;
			}
			case vk::ImageLayout::eColorAttachmentOptimal:
			{
				p_out_access_flags = vk::AccessFlagBits2::eColorAttachmentRead | vk::AccessFlagBits2::eColorAttachmentWrite;
				p_out_stage_mask   = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
			}
			case vk::ImageLayout::eDepthStencilAttachmentOptimal:
			{
				p_out_access_flags = vk::AccessFlagBits2::eDepthStencilAttachmentRead | vk::AccessFlagBits2::eDepthStencilAttachmentWrite;
				p_out_stage_mask   = vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests;
			}
			case vk::ImageLayout::eDepthStencilReadOnlyOptimal:
			{
				p_out_access_flags = vk::AccessFlagBits2::eDepthStencilAttachmentRead;
				p_out_stage_mask   = vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests;
			}
			case vk::ImageLayout::eShaderReadOnlyOptimal:
			{
				p_out_access_flags = vk::AccessFlagBits2::eShaderRead;
				p_out_stage_mask = vk::PipelineStageFlagBits2::eVertexShader | vk::PipelineStageFlagBits2::eFragmentShader | vk::PipelineStageFlagBits2::eComputeShader |
								   vk::PipelineStageFlagBits2::eGeometryShader | vk::PipelineStageFlagBits2::eTessellationControlShader |
								   vk::PipelineStageFlagBits2::eTessellationEvaluationShader;
			}
			case vk::ImageLayout::eTransferSrcOptimal:
			{
				p_out_access_flags = vk::AccessFlagBits2::eTransferRead;
				p_out_stage_mask   = vk::PipelineStageFlagBits2::eTransfer;
			}
			case vk::ImageLayout::eTransferDstOptimal:
			{
				p_out_access_flags = vk::AccessFlagBits2::eTransferWrite;
				p_out_stage_mask   = vk::PipelineStageFlagBits2::eTransfer;
			}
			case vk::ImageLayout::eDepthAttachmentOptimal:
			{
				p_out_access_flags = vk::AccessFlagBits2::eDepthStencilAttachmentRead | vk::AccessFlagBits2::eDepthStencilAttachmentWrite;
				p_out_stage_mask   = vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests;
			}
			case vk::ImageLayout::eDepthReadOnlyOptimal:
			{
				p_out_access_flags = vk::AccessFlagBits2::eDepthStencilAttachmentRead;
				p_out_stage_mask   = vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests;
			}
			case vk::ImageLayout::eStencilAttachmentOptimal:
			{
				p_out_access_flags = vk::AccessFlagBits2::eDepthStencilAttachmentRead | vk::AccessFlagBits2::eDepthStencilAttachmentWrite;
				p_out_stage_mask   = vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests;
			}
			case vk::ImageLayout::eStencilReadOnlyOptimal:
			{
				p_out_access_flags = vk::AccessFlagBits2::eDepthStencilAttachmentRead;
				p_out_stage_mask   = vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests;
			}
			default:
			{
				p_out_access_flags = vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite;
				p_out_stage_mask   = vk::PipelineStageFlagBits2::eAllCommands;
			}
		}
	}

	constexpr auto getVulkanShaderStages(EShaderStageFlags p_stages) -> vk::ShaderStageFlags
	{
		vk::ShaderStageFlags out_flags{0u};
		if (p_stages & EShaderStageBits::eVertex)
			out_flags |= vk::ShaderStageFlagBits::eVertex;
		if (p_stages & EShaderStageBits::ePixel)
			out_flags |= vk::ShaderStageFlagBits::eFragment;
		if (p_stages & EShaderStageBits::eCompute)
			out_flags |= vk::ShaderStageFlagBits::eCompute;
		if (p_stages & EShaderStageBits::eGeometry)
			out_flags |= vk::ShaderStageFlagBits::eGeometry;
		if (p_stages & EShaderStageBits::eTessellationControl)
			out_flags |= vk::ShaderStageFlagBits::eTessellationControl;
		if (p_stages & EShaderStageBits::eTessellationEvaluation)
			out_flags |= vk::ShaderStageFlagBits::eTessellationEvaluation;
		if (p_stages & EShaderStageBits::eMesh)
			out_flags |= vk::ShaderStageFlagBits::eMeshEXT;
		if (p_stages & EShaderStageBits::eTask)
			out_flags |= vk::ShaderStageFlagBits::eTaskEXT;

		return out_flags;
	}

	constexpr auto getVulkanImageViewType(vk::ImageType p_type) -> vk::ImageViewType
	{
		switch (p_type)
		{
			case vk::ImageType::e1D: return vk::ImageViewType::e1D;
			case vk::ImageType::e2D: return vk::ImageViewType::e2D;
			case vk::ImageType::e3D: return vk::ImageViewType::e3D;
		}
		return vk::ImageViewType::e2D;
	}
}
