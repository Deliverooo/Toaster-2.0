/*!
 * @file gpu_context.cpp
 */

#include "gpu_context.hpp"

#include <set>
#include <sstream>
#include <GLFW/glfw3.h>

#include "gl/gl_gpu_context.hpp"

namespace toaster::gpu
{
	GPUContext *GPUContext::create(GLFWwindow *p_window)
	{
		return new GLGPUContext(p_window);
	}
}
