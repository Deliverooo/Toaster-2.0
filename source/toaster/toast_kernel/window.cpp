#include "window.hpp"

#include "swapchain.hpp"
#include "gpu_context.hpp"

#include <GLFW/glfw3.h>

#include "toast_assert.h"

namespace toaster
{
	static bool s_glfwInitialized = false;

	void Window::initWindowingAPI()
	{
		if (!s_glfwInitialized)
		{
			const bool init_result = glfwInit();
			TST_ASSERT_MSG(init_result, "glfw initialization failed!");

			s_glfwInitialized = true;
		}
	}

	void Window::shutdownWindowingAPI()
	{
		TST_ASSERT_MSG(s_glfwInitialized, "Attempted to shutdown windowing API before initializing it!");

		glfwTerminate();
	}

	Window::Window(uint32 p_width, uint32 p_height, std::string p_title)
	{
		// Tells GLFW not to create an OpenGL context.
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		// Hides the window during creation, as to not have a blank white screen
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

		m_callbackData.width  = p_width;
		m_callbackData.height = p_height;
		m_callbackData.title  = p_title;

		m_window = glfwCreateWindow(static_cast<int32>(p_width), static_cast<int32>(p_height), p_title.c_str(), nullptr, nullptr);

		glfwSetWindowUserPointer(m_window, &m_callbackData);

		int32 width;
		int32 height;
		glfwGetFramebufferSize(m_window, &width, &height);
		m_callbackData.width  = width;
		m_callbackData.height = width;

		m_gpuContext = new gpu::GPUContext();

		if (glfwCreateWindowSurface(m_gpuContext->getInstance(), m_window, nullptr, reinterpret_cast<VkSurfaceKHR *>(&m_windowSurface)) != VK_SUCCESS)
		{
			TST_ASSERT_MSG(false, "Failed to create window surface!");
		}

		m_gpuContext->init(m_windowSurface);

		m_swapchain = new gpu::Swapchain(m_gpuContext, m_windowSurface);
		m_swapchain->create(p_width, p_height);

		showWindow();
	}

	Window::~Window()
	{
		m_swapchain->destroy();
		delete m_swapchain;

		m_gpuContext->getInstance().destroySurfaceKHR(m_windowSurface);
		delete m_gpuContext;

		glfwDestroyWindow(m_window);
	}

	void Window::beginFrame()
	{
		m_swapchain->beginFrame();
	}

	void Window::processEvents()
	{
		glfwPollEvents();

		int width;
		int height;
		glfwGetWindowSize(m_window, &width, &height);

		if (m_callbackData.width != width || m_callbackData.height != height)
		{
			m_callbackData.width  = width;
			m_callbackData.height = height;

			m_swapchain->onResize(width, height);
		}
	}

	void Window::endFrame()
	{
		m_swapchain->present();
	}

	void Window::showWindow()
	{
		glfwShowWindow(m_window);
	}

	void Window::hideWindow()
	{
		glfwHideWindow(m_window);
	}

	uint32 Window::getWidth() const
	{
		return m_callbackData.width;
	}

	uint32 Window::getHeight() const
	{
		return m_callbackData.height;
	}

	const std::string &Window::getTitle() const
	{
		return m_callbackData.title;
	}

	gpu::GPUContext *Window::getGPUContext() const
	{
		return m_gpuContext;
	}

	gpu::Swapchain *Window::getSwapchain() const
	{
		return m_swapchain;
	}

	GLFWwindow *Window::getNativeWindow() const
	{
		return m_window;
	}
}
