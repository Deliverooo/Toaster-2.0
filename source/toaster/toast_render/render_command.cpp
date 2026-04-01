#include "render_command.hpp"

#include "toast_gpu/gpu_api.hpp"

namespace toaster
{
	static std::unique_ptr<gpu::IGPUAPI> s_gpuAPI = nullptr;

	void RenderCommand::init()
	{
		s_gpuAPI = gpu::IGPUAPI::create();
	}

	void RenderCommand::clearColour(float32 p_r, float32 p_g, float32 p_b, float32 p_a)
	{
		s_gpuAPI->clearColour(p_r, p_g, p_b, p_a);
	}

	void RenderCommand::clear()
	{
		s_gpuAPI->clear();
	}

	void RenderCommand::setViewport(int32 p_x, int32 p_y, int32 p_width, int32 p_height)
	{
		s_gpuAPI->setViewport(p_x, p_y, p_width, p_height);
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
