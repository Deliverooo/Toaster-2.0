#pragma once

#include "toast_gpu/vk/vk_render_pass.hpp"
#include "toast_gpu/vk/vk_vertex_buffer.hpp"
#include "toast_gpu/vk/vk_index_buffer.hpp"
#include "toast_gpu/vk/vk_material.hpp"
#include "toast_gpu/vk/vk_render_attachment.hpp"

#include <glm/glm.hpp>

namespace toaster
{
	// Static interface class
	class Renderer final
	{
	public:
		static void beginRendering(const gpu::RenderingInfo &       p_rendering_info, const vk::raii::CommandBuffer &p_command_buffer, uint32 p_frame_index,
								   const RefPtr<gpu::VKRenderPass> &p_render_pass);
		static void endRendering(const gpu::RenderingInfo &p_rendering_info, const vk::raii::CommandBuffer &p_command_buffer);

		static void renderGeometry(const vk::raii::CommandBuffer &    p_command_buffer, uint32 p_frame_index, const RefPtr<gpu::VKPipeline> &p_pipeline,
								   const RefPtr<gpu::VKVertexBuffer> &p_vertex_buffer, const RefPtr<gpu::VKIndexBuffer> &p_index_buffer, uint32 p_index_count,
								   const RefPtr<gpu::VKMaterial> &    p_material, const glm::mat4 &p_transform);
	};
}
