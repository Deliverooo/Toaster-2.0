/*!
 * @file gpu_context.cpp
 */

#include "gpu_context.hpp"

#include <set>
#include <sstream>

#include "vk/vk_gpu_context.hpp"

namespace toaster::gpu
{
	IGPUContext *IGPUContext::create(GLFWwindow *p_window)
	{
		return new VKGPUContext(p_window);
	}
}
