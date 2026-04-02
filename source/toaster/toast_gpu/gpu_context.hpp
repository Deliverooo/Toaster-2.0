/*!
 * @file gpu_context.hpp
 */
#pragma once

struct GLFWwindow;

namespace toaster::gpu
{
	class IGPUContext
	{
	public:
		static IGPUContext *create(GLFWwindow *p_window);
		virtual             ~IGPUContext() noexcept = default;
	};
}
