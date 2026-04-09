#include "renderer.hpp"
#include "toast_gpu/vk/vk_gpu_context.hpp"

namespace toaster
{
	void Renderer::beginRendering(const vk::RenderingInfo &        p_rendering_info, vk::raii::CommandBuffer &p_command_buffer, uint32 p_frame_index,
								  const RefPtr<gpu::VKRenderPass> &p_render_pass)
	{
		vk::Extent2D rendering_extent{p_rendering_info.renderArea.extent};

		vk::Viewport viewport{0.0f, 0.0f, static_cast<float32>(rendering_extent.width), static_cast<float32>(rendering_extent.height), 0.0f, 1.0f};
		vk::Rect2D   scissor{vk::Offset2D{0, 0}, rendering_extent};

		p_command_buffer.beginRendering(p_rendering_info);
		p_command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, p_render_pass->getPipeline()->getPipeline());
		p_command_buffer.setViewport(0, viewport);
		p_command_buffer.setScissor(0, scissor);

		const auto descriptor_sets = p_render_pass->getDescriptorSets(p_frame_index);
		p_command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, p_render_pass->getPipeline()->getPipelineLayout(), 0, descriptor_sets, nullptr);
	}

	void Renderer::endRendering(vk::raii::CommandBuffer &p_command_buffer)
	{
		p_command_buffer.endRendering();
	}
}
