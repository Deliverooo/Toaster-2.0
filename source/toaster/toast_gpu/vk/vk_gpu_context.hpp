#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#ifdef WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#endif

#include <GLFW/glfw3native.h>

#include <vulkan/vulkan.hpp>
#include "../gpu_context.hpp"

#include "toast_lib/string.hpp"
#include "toast_lib/system_types.h"

#include <unordered_set>

#ifndef NDEBUG
#define ENABLE_VALIDATION_LAYERS 1
#else
#define ENABLE_VALIDATION_LAYERS 0
#endif


#define TST_VK_CHECK_RESULT(f) \
{\
	vk::Result res = (f);\
	assert(res == vk::Result::eSuccess);\
}

namespace toaster::gpu
{
	// struct QueueFamilyIndices
	// {
	// 	int32 graphics = -1;
	// 	int32 compute  = -1;
	// 	int32 transfer = -1;
	// 	int32 present  = -1;
	// };

	class VKGPUContext : public IGPUContext
	{
	public:
		VKGPUContext(GLFWwindow *p_window);
		~VKGPUContext() override;

	private:

		vk::Instance m_vulkanInstance{nullptr};
	};
}
