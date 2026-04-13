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
		static auto create(GLFWwindow *p_window) -> IGPUContext *;
		virtual     ~IGPUContext() noexcept = default;
	};
}
