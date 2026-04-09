#include "renderer.hpp"
#include "toast_gpu/vk/vk_gpu_context.hpp"

namespace toaster
{
	void Renderer::beginRendering(const vk::RenderingInfo &        p_rendering_info, const vk::raii::CommandBuffer &p_command_buffer, uint32 p_frame_index,
								  const RefPtr<gpu::VKRenderPass> &p_render_pass)
	{
		const vk::Extent2D rendering_extent{p_rendering_info.renderArea.extent};

		const vk::Viewport viewport{0.0f, 0.0f, static_cast<float32>(rendering_extent.width), static_cast<float32>(rendering_extent.height), 0.0f, 1.0f};
		const vk::Rect2D   scissor{vk::Offset2D{0, 0}, rendering_extent};

		p_command_buffer.beginRendering(p_rendering_info);
		p_command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, p_render_pass->getPipeline()->getPipeline());
		p_command_buffer.setViewport(0, viewport);
		p_command_buffer.setScissor(0, scissor);

		p_render_pass->update(p_frame_index);

		const auto descriptor_sets = p_render_pass->getDescriptorSets(p_frame_index);
		p_command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, p_render_pass->getPipeline()->getPipelineLayout(), p_render_pass->getStartSetIndex(),
											descriptor_sets, nullptr);
	}

	void Renderer::endRendering(const vk::raii::CommandBuffer &p_command_buffer)
	{
		p_command_buffer.endRendering();
	}

	void Renderer::renderGeometry(const vk::raii::CommandBuffer &    p_command_buffer, uint32 p_frame_index, const RefPtr<gpu::VKPipeline> &p_pipeline,
								  const RefPtr<gpu::VKVertexBuffer> &p_vertex_buffer, const RefPtr<gpu::VKIndexBuffer> &p_index_buffer, uint32 p_index_count,
								  const RefPtr<gpu::VKMaterial> &    p_material, const glm::mat4 &p_transform)
	{
		// Push the constants
		p_command_buffer.pushConstants<glm::mat4>(p_pipeline->getPipelineLayout(), vk::ShaderStageFlagBits::eVertex, 0, p_transform);

		// Bind the material descriptor set (0)
		vk::DescriptorSet material_descriptor_set{p_material->getDescriptorSet(p_frame_index)};
		p_command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, p_pipeline->getPipelineLayout(), 0, material_descriptor_set, {});

		// Bind the vertex and index buffers
		p_vertex_buffer->bind(p_command_buffer);
		p_index_buffer->bind(p_command_buffer, vk::IndexType::eUint16);

		// Finally, draw indexed :)
		p_command_buffer.drawIndexed(p_index_count, 1, 0, 0, 0);
	}
}
