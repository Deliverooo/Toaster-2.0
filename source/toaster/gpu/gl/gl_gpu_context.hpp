/*!
 * @file gl_gpu_context.hpp
 */
#pragma once

#include "gpu_context.hpp"

namespace toaster::gpu
{
	class GLGPUContext final : public GPUContext
	{
	public:
		explicit GLGPUContext(GLFWwindow *p_window);
		~GLGPUContext() override = default;

	private:
		GLFWwindow *m_window{nullptr};
	};
}
