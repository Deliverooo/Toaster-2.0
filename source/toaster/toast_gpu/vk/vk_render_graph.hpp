#pragma once

#include <vulkan/vulkan_raii.hpp>
#include "toast_lib/core_basic.hpp"

namespace toaster::gpu
{
	class VKGPUContext;

	class VKRenderGraph
	{
	public:
		VKRenderGraph(VKGPUContext *p_ctx);

		void compile();
		void execute(vk::raii::CommandBuffer &p_command_buffer);

	private:
		VKGPUContext *m_ctx{nullptr};
	};
}
