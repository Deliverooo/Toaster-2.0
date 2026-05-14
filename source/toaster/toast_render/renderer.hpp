#pragma once

#include "../toaster_macros.hpp"

#include "toast_gpu/vk/vk_compute_pass.hpp"
#include "toast_gpu/vk/vk_index_buffer.hpp"
#include "toast_gpu/vk/vk_material.hpp"
#include "toast_gpu/vk/vk_render_attachment.hpp"
#include "toast_gpu/vk/vk_render_pass.hpp"
#include "toast_gpu/vk/vk_vertex_buffer.hpp"

#include <glm/glm.hpp>

#include "globals.hpp"
#include "toast_gpu/vk/vk_logical_device.hpp"
#include "toast_gpu/vk/vk_mesh.hpp"

namespace toaster::render
{
	TST_API auto createEnvironmentMap(gpu::VKLogicalDevice *p_device, const Globals *p_globals, const io::filesystem::Path &p_path) -> RefPtr<gpu::VKTexture3D>;

	TST_API auto renderFullscreenQuad(const Globals *                p_globals, const vk::raii::CommandBuffer & p_command_buffer, uint32 p_frame_index,
									  const RefPtr<gpu::VKPipeline> &p_pipeline, const RefPtr<gpu::VKMaterial> &p_material) -> void;
}
