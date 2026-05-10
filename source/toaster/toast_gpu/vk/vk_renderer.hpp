#pragma once
#include "vk_compute_pass.hpp"
#include "vk_index_buffer.hpp"
#include "vk_material.hpp"
#include "vk_mesh.hpp"
#include "vk_render_attachment.hpp"
#include "vk_render_pass.hpp"
#include "vk_vertex_buffer.hpp"

namespace toaster::gpu::render
{
	TST_GPU_API auto beginRendering(const RenderingInfo &       p_rendering_info, const vk::raii::CommandBuffer &p_command_buffer, uint32 p_frame_index,
									const RefPtr<VKRenderPass> &p_render_pass) -> void;
	TST_GPU_API auto endRendering(const RenderingInfo &p_rendering_info, const vk::raii::CommandBuffer &p_command_buffer) -> void;

	TST_GPU_API auto beginCompute(const vk::raii::CommandBuffer &p_command_buffer, uint32 p_frame_index, const RefPtr<VKComputePass> &p_compute_pass) -> void;
	TST_GPU_API auto dispatchCompute(const vk::raii::CommandBuffer &p_command_buffer, uint32 p_frame_index, const RefPtr<VKComputePass> &p_compute_pass,
									 const RefPtr<VKMaterial> &     p_material, uint32       p_work_group_x, uint32 p_work_group_y, uint32 p_work_group_z) -> void;
	TST_GPU_API auto endCompute(const vk::raii::CommandBuffer &p_command_buffer, uint32 p_frame_index, const RefPtr<VKComputePass> &p_compute_pass) -> void;

	TST_GPU_API auto renderGeometry(const vk::raii::CommandBuffer &p_command_buffer, uint32                      p_frame_index, const RefPtr<VKPipeline> &p_pipeline,
									const RefPtr<VKVertexBuffer> & p_vertex_buffer, const RefPtr<VKIndexBuffer> &p_index_buffer, uint32                   p_index_count,
									const RefPtr<VKMaterial> &     p_material, const glm::mat4 &                 p_transform) -> void;

	TST_GPU_API auto renderFullscreenQuad(const vk::raii::CommandBuffer &p_command_buffer, uint32 p_frame_index, const RefPtr<VKPipeline> &p_pipeline,
										  const RefPtr<VKMaterial> &     p_material) -> void;

	TST_GPU_API auto renderMesh(const vk::raii::CommandBuffer &p_command_buffer, uint32     p_frame_index, const RefPtr<VKMesh> &p_mesh, uint32 p_submesh_index,
								const RefPtr<VKPipeline> &     p_pipeline, const glm::mat4 &p_transform) -> void;

	TST_GPU_API auto renderMesh(const vk::raii::CommandBuffer &p_command_buffer, uint32     p_frame_index, const RefPtr<VKMesh> &  p_mesh, uint32 p_submesh_index,
								const RefPtr<VKPipeline> &     p_pipeline, const glm::mat4 &p_transform, const RefPtr<VKMaterial> &p_override_material) -> void;
}
