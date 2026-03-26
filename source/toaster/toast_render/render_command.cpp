#include "render_command.hpp"

#include "toast_gpu/gpu_api.hpp"

namespace toaster
{
	static std::unique_ptr<gpu::IGPUAPI> s_gpuAPI = nullptr;

	void RenderCommand::init()
	{
		s_gpuAPI = gpu::IGPUAPI::create();
	}

	void RenderCommand::clearColour(const glm::vec4 &p_colour)
	{
		s_gpuAPI->clearColour(p_colour);
	}

	void RenderCommand::clear()
	{
		s_gpuAPI->clear();
	}

	void RenderCommand::setViewport(const glm::vec4 &p_viewport)
	{
		s_gpuAPI->setViewport(p_viewport);
	}

	void RenderCommand::drawIndexed(const RefPtr<gpu::IVertexArray> &p_vertex_array, uint32 p_index_count)
	{
		s_gpuAPI->drawIndexed(p_vertex_array, p_index_count);
	}

	void RenderCommand::drawIndexedBaseVertex(const RefPtr<gpu::IVertexArray> &p_vertex_array, uint32 p_index_count, uint32 p_base_index, uint32 p_base_vertex)
	{
		s_gpuAPI->drawIndexedBaseVertex(p_vertex_array, p_index_count, p_base_index, p_base_vertex);
	}
}
