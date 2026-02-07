#include "render_command.hpp"

#include "gpu_api.hpp"

namespace toaster
{
	static std::unique_ptr<gpu::GPUAPI> s_gpuAPI = nullptr;

	void RenderCommand::init()
	{
		s_gpuAPI = gpu::GPUAPI::create();
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

	void RenderCommand::drawIndexed(const RefPtr<gpu::VertexArray> &p_vertex_array)
	{
		s_gpuAPI->drawIndexed(p_vertex_array);
	}
}
