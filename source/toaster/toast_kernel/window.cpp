#include "window.hpp"

#include "gpu_context.hpp"

#if USE_VULKAN_BACKEND
#include "swapchain.hpp"
#define GLFW_INCLUDE_VULKAN
#endif
#include <GLFW/glfw3.h>

#include "toast_assert.h"

#include "events/key_event.hpp"
#include "events/mouse_event.hpp"
#include "events/window_event.hpp"

namespace toaster
{
	static bool s_glfwInitialized = false;

	static void glfwErrorCallback(int error, const char *description)
	{
		LOG_ERROR("GLFW error: ({}): {}", error, description);
	}

	void Window::initWindowingAPI()
	{
		if (!s_glfwInitialized)
		{
			const bool init_result = glfwInit();
			TST_ASSERT_MSG(init_result, "glfw initialization failed!");

			glfwSetErrorCallback(glfwErrorCallback);

			s_glfwInitialized = true;
		}
	}

	void Window::shutdownWindowingAPI()
	{
		TST_ASSERT_MSG(s_glfwInitialized, "Attempted to shutdown windowing API before initializing it!");

		glfwTerminate();
	}

	Window::Window(uint32 p_width, uint32 p_height, const std::string &p_title)
	{
		#if USE_VULKAN_BACKEND
		// Tells GLFW not to create an OpenGL context.
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		#else

		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
		#endif

		// Hides the window during creation, as to not have a blank white screen
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

		m_callbackData.width  = p_width;
		m_callbackData.height = p_height;
		m_callbackData.title  = p_title;

		m_window = glfwCreateWindow(static_cast<int32>(p_width), static_cast<int32>(p_height), p_title.c_str(), nullptr, nullptr);

		m_gpuContext = gpu::GPUContext::create(m_window);

		glfwSetWindowUserPointer(m_window, &m_callbackData);

		#if USE_VULKAN_BACKEND
		m_swapchain = new gpu::Swapchain(m_gpuContext, m_windowSurface); m_swapchain->create(p_width, p_height);
		#endif

		glfwSetWindowSizeCallback(m_window, [](GLFWwindow *window, const int width, const int height)
		{
			auto &data = *(static_cast<GLFWCallbackData *>(glfwGetWindowUserPointer(window)));

			WindowResizedEvent event(width, height);
			data.eventCallback(event);
			data.width  = width;
			data.height = height;
		});

		glfwSetWindowCloseCallback(m_window, [](GLFWwindow *window)
		{
			const auto &data = *(static_cast<GLFWCallbackData *>(glfwGetWindowUserPointer(window)));

			WindowClosedEvent event;
			data.eventCallback(event);
		});

		glfwSetKeyCallback(m_window, [](GLFWwindow *window, int key, int scancode, int action, int mods)
		{
			const auto &data = *(static_cast<GLFWCallbackData *>(glfwGetWindowUserPointer(window)));

			switch (action)
			{
				case GLFW_PRESS:
				{
					KeyPressedEvent event(static_cast<KeyCode>(key), 0);
					data.eventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent event(static_cast<KeyCode>(key));
					data.eventCallback(event);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent event(static_cast<KeyCode>(key), 1);
					data.eventCallback(event);
					break;
				}
				default:
					break;
			}
		});

		glfwSetCharCallback(m_window, [](GLFWwindow *window, uint32_t codepoint)
		{
			const auto &data = *(static_cast<GLFWCallbackData *>(glfwGetWindowUserPointer(window)));

			KeyTypedEvent event(static_cast<KeyCode>(codepoint));
			data.eventCallback(event);
		});

		glfwSetMouseButtonCallback(m_window, [](GLFWwindow *window, int button, int action, int mods)
		{
			const auto &data = *(static_cast<GLFWCallbackData *>(glfwGetWindowUserPointer(window)));

			switch (action)
			{
				case GLFW_PRESS:
				{
					MouseButtonPressEvent event(static_cast<MouseButton>(button));
					data.eventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleaseEvent event(static_cast<MouseButton>(button));
					data.eventCallback(event);
					break;
				}
				default: break;
			}
		});

		glfwSetScrollCallback(m_window, [](GLFWwindow *window, double xOffset, double yOffset)
		{
			const auto &data = *(static_cast<GLFWCallbackData *>(glfwGetWindowUserPointer(window)));

			MouseScrollEvent event(static_cast<float>(xOffset), static_cast<float>(yOffset));
			data.eventCallback(event);
		});

		glfwSetCursorPosCallback(m_window, [](GLFWwindow *window, double x, double y)
		{
			const auto &   data = *(static_cast<GLFWCallbackData *>(glfwGetWindowUserPointer(window)));
			MouseMoveEvent event(static_cast<float>(x), static_cast<float>(y));
			data.eventCallback(event);
		});

		glfwSetWindowIconifyCallback(m_window, [](GLFWwindow *window, int iconified)
		{
			const auto &data = *(static_cast<GLFWCallbackData *>(glfwGetWindowUserPointer(window)));

			WindowMinimizedEvent event(static_cast<bool>(iconified));
			data.eventCallback(event);
		});

		showWindow();
	}

	Window::~Window()
	{
		#if USE_VULKAN_BACKEND

		m_swapchain->destroy(); delete m_swapchain; m_gpuContext->getInstance().destroySurfaceKHR(m_windowSurface);
		#endif
		delete m_gpuContext;

		glfwDestroyWindow(m_window);
	}

	void Window::beginFrame()
	{
		#if USE_VULKAN_BACKEND
		m_swapchain->beginFrame();
		#endif
	}

	void Window::processEvents()
	{
		glfwPollEvents();

		#if USE_VULKAN_BACKEND
		int width; int height; glfwGetWindowSize(m_window, &width, &height); if (m_callbackData.width != width || m_callbackData.height != height)
		{
			m_callbackData.width  = width;
			m_callbackData.height = height;

			m_swapchain->onResize(width, height);
		}
		#endif
	}

	void Window::endFrame()
	{
		#if USE_VULKAN_BACKEND
		m_swapchain->present();
		#else
		glfwSwapBuffers(m_window);
		#endif
	}

	void Window::showWindow()
	{
		glfwShowWindow(m_window);
	}

	void Window::hideWindow()
	{
		glfwHideWindow(m_window);
	}

	void Window::setEventCallback(const EventCallbackFn &p_callback)
	{
		m_callbackData.eventCallback = p_callback;
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

	#if USE_VULKAN_BACKEND
	gpu::Swapchain *Window::getSwapchain() const
	{
		return m_swapchain;
	}
	#endif

	GLFWwindow *Window::getNativeWindow() const
	{
		return m_window;
	}
}
