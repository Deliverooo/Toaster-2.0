/*!
 * @file gpu_context.hpp
 */
#pragma once

struct GLFWwindow;

namespace toaster::gpu
{
	class GPUContext
	{
	public:
		static GPUContext *create(GLFWwindow *p_window);
		virtual            ~GPUContext() = default;
	};
}
