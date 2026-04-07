#include "vk_render_graph.hpp"

namespace toaster::gpu
{
	VKRenderGraph::VKRenderGraph(VKGPUContext *p_ctx) : m_ctx(p_ctx)
	{
	}

	void VKRenderGraph::compile()
	{
	}

	void VKRenderGraph::execute(vk::raii::CommandBuffer &p_command_buffer)
	{
	}
}
