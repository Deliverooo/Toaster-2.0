#include "vk_swapchain.hpp"

namespace toaster::gpu
{
	VKSwapchain::VKSwapchain(VKGPUContext *p_ctx, GLFWwindow *p_window) : m_ctx(p_ctx), m_window(p_window)
	{
	}

	VKSwapchain::~VKSwapchain()
	{
	}
}
