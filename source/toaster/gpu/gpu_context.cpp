#include "gpu_context.hpp"
#include "logging.hpp"

#include <set>
#include <sstream>
#include <GLFW/glfw3.h>

#include "toast_assert.h"

#if USE_OPENGL_BACKEND
#include "gl/gl_gpu_context.hpp"
#elif USE_VULKAN_BACKEND
#include "vk/vk_gpu_context.hpp"
#endif

namespace toaster::gpu
{
	GPUContext *GPUContext::create(GLFWwindow *p_window)
	{
		return new GLGPUContext(p_window);
	}
}
