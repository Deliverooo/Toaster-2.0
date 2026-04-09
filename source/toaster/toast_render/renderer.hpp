#pragma once

#include "toast_gpu/vk/vk_render_pass.hpp"

namespace toaster
{
	// Static interface class
	class Renderer final
	{
	public:
		static void beginRendering(const vk::RenderingInfo &        p_rendering_info, vk::raii::CommandBuffer &p_command_buffer, uint32 p_frame_index,
								   const RefPtr<gpu::VKRenderPass> &p_render_pass);
		static void endRendering(vk::raii::CommandBuffer &p_command_buffer);
	};
}
