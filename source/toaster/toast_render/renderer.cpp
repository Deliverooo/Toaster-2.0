#include "renderer.hpp"

namespace toaster
{
	void Renderer::beginRenderPass(vk::raii::CommandBuffer &p_command_buffer, uint32 p_frame_index, const gpu::RenderPassBeginInfo &begin_info)
	{
		std::vector<vk::RenderingAttachmentInfo> colour_attachments{};
		vk::RenderingAttachmentInfo              depth_attachment{};

		for (auto &attachment: begin_info.attachments)
		{
			if (attachment.type == gpu::ERenderAttachmentType::eColour)
			{
				vk::RenderingAttachmentInfo &colour_attachment = colour_attachments.emplace_back();
				colour_attachment.clearValue                   = attachment.clearValue;
				colour_attachment.imageLayout                  = vk::ImageLayout::eColorAttachmentOptimal;
				colour_attachment.imageView                    = attachment.targetImage;
				colour_attachment.loadOp                       = vk::AttachmentLoadOp::eClear;
				colour_attachment.storeOp                      = vk::AttachmentStoreOp::eStore;
			}
		}

	}

	void Renderer::endRenderPass(vk::raii::CommandBuffer &p_command_buffer)
	{
		p_command_buffer.endRendering();
	}
}
