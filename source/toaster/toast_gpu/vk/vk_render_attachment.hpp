#pragma once

#include <vulkan/vulkan_raii.hpp>
#include "toast_lib/core_basic.hpp"

namespace toaster::gpu
{
	class VKGPUContext;

	struct RenderAttachmentSpecInfo
	{
		uint32 width{0u};
		uint32 height{0u};

		std::vector<vk::Format> colourAttachmentFormats;
		vk::Format              depthAttachmentFormat{vk::Format::eUndefined};
	};

	class VKRenderAttachment
	{
	public:
		VKRenderAttachment(VKGPUContext *p_ctx, const RenderAttachmentSpecInfo& p_spec_info);

	private:
		VKGPUContext *m_ctx{nullptr};

		RenderAttachmentSpecInfo m_specInfo;

		std::vector<vk::raii::Image>        m_colourAttachments;
		std::vector<vk::raii::ImageView>    m_colourAttachmentViews;
		std::vector<vk::raii::DeviceMemory> m_colourAttachmentMemories;

		vk::raii::Image        m_depthAttachment{nullptr};
		vk::raii::ImageView    m_depthAttachmentView{nullptr};
		vk::raii::DeviceMemory m_depthAttachmentMemory{nullptr};
	};
}
