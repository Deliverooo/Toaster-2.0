#pragma once

#include "vk_gpu_context.hpp"

namespace toaster::gpu
{
	class VKSwapchain
	{
	public:
		VKSwapchain(VKGPUContext *p_ctx, GLFWwindow *p_window);
		~VKSwapchain();

	private:
		VKGPUContext *m_ctx;

		GLFWwindow *         m_window{nullptr}; // Used as a reference to create the window surface
		vk::raii::SurfaceKHR m_surface{nullptr};
	};
}
