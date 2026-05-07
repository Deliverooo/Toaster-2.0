#pragma once

#include "../toaster_macros.hpp"

#include "toast_gpu/vk/vk_compute_pass.hpp"
#include "toast_gpu/vk/vk_index_buffer.hpp"
#include "toast_gpu/vk/vk_material.hpp"
#include "toast_gpu/vk/vk_render_attachment.hpp"
#include "toast_gpu/vk/vk_render_pass.hpp"
#include "toast_gpu/vk/vk_vertex_buffer.hpp"

#include <glm/glm.hpp>

#include "toast_gpu/vk/vk_mesh.hpp"

namespace toaster::render
{
	TST_API auto beginRendering(const gpu::RenderingInfo &       p_rendering_info, const vk::raii::CommandBuffer &p_command_buffer, uint32 p_frame_index,
								const RefPtr<gpu::VKRenderPass> &p_render_pass) -> void;
	TST_API auto endRendering(const gpu::RenderingInfo &p_rendering_info, const vk::raii::CommandBuffer &p_command_buffer) -> void;

	TST_API auto beginCompute(const vk::raii::CommandBuffer &p_command_buffer, uint32 p_frame_index, const RefPtr<gpu::VKComputePass> &p_compute_pass) -> void;
	TST_API auto dispatchCompute(const vk::raii::CommandBuffer &p_command_buffer, uint32 p_frame_index, const RefPtr<gpu::VKComputePass> &p_compute_pass,
								 const RefPtr<gpu::VKMaterial> &p_material, uint32       p_work_group_x, uint32 p_work_group_y, uint32 p_work_group_z) -> void;
	TST_API auto endCompute(const vk::raii::CommandBuffer &p_command_buffer, uint32 p_frame_index, const RefPtr<gpu::VKComputePass> &p_compute_pass) -> void;

	TST_API auto renderGeometry(const vk::raii::CommandBuffer &    p_command_buffer, uint32 p_frame_index, const RefPtr<gpu::VKPipeline> &p_pipeline,
								const RefPtr<gpu::VKVertexBuffer> &p_vertex_buffer, const RefPtr<gpu::VKIndexBuffer> &p_index_buffer, uint32 p_index_count,
								const RefPtr<gpu::VKMaterial> &    p_material, const glm::mat4 &p_transform) -> void;

	TST_API auto renderFullscreenQuad(const vk::raii::CommandBuffer &p_command_buffer, uint32 p_frame_index, const RefPtr<gpu::VKPipeline> &p_pipeline,
									  const RefPtr<gpu::VKMaterial> &p_material) -> void;

	TST_API auto renderMesh(const vk::raii::CommandBuffer &p_command_buffer, uint32     p_frame_index, const RefPtr<gpu::VKMesh> &p_mesh, uint32 p_submesh_index,
							const RefPtr<gpu::VKPipeline> &p_pipeline, const glm::mat4 &p_transform) -> void;

	TST_API auto renderMesh(const vk::raii::CommandBuffer &p_command_buffer, uint32     p_frame_index, const RefPtr<gpu::VKMesh> &  p_mesh, uint32 p_submesh_index,
							const RefPtr<gpu::VKPipeline> &p_pipeline, const glm::mat4 &p_transform, const RefPtr<gpu::VKMaterial> &p_override_material) -> void;
}
