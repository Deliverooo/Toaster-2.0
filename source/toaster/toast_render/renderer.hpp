#pragma once

#include "toast_gpu/vk/vk_render_pass.hpp"

namespace toaster
{
	// Static interface class
	class Renderer final
	{
	public:
		static void beginRenderPass(vk::raii::CommandBuffer &p_command_buffer, uint32 p_frame_index, const gpu::RenderPassBeginInfo &begin_info);
		static void endRenderPass(vk::raii::CommandBuffer &p_command_buffer);

	};
}
