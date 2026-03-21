/*!
 * @file gl_gpu_context.hpp
 */
#pragma once

#include "toaster/toast_gpu/gpu_context.hpp"

namespace toaster::gpu
{
	class GLGPUContext final : public IGPUContext
	{
	public:
		explicit GLGPUContext(GLFWwindow *p_window);
		~GLGPUContext() override = default;

	private:
		GLFWwindow *m_window{nullptr};
	};
}
