#pragma once

#include "gpu_context.hpp"

namespace toaster::gpu
{
	class GLGPUContext : public GPUContext
	{
	public:
		GLGPUContext(GLFWwindow *p_window);
		~GLGPUContext() override;

	private:
		GLFWwindow *m_window{nullptr};
	};
}
