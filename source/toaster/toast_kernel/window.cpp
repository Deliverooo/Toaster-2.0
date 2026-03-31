#include "window.hpp"

#include "toast_gpu/gpu_context.hpp"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <dwmapi.h>
#include <GLFW/glfw3native.h>

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

#include <stb/stb_image.h>

#include "application.hpp"
#include "toast_lib/logging.hpp"
#include "toast_lib/toast_assert.h"

#include "toast_lib/events/key_event.hpp"
#include "toast_lib/events/mouse_event.hpp"
#include "toast_lib/events/window_event.hpp"

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

	Window::Window(const WindowCreateInfo &p_create_info)
	{
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		// Hides the window during creation, as to not have a blank white screen
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

		glfwWindowHint(GLFW_SAMPLES, 4);

		m_callbackData.width  = p_create_info.width;
		m_callbackData.height = p_create_info.height;
		m_callbackData.title  = p_create_info.title;

		m_window = glfwCreateWindow(static_cast<int32>(p_create_info.width), static_cast<int32>(p_create_info.height), p_create_info.title.c_str(), nullptr, nullptr);

		m_gpuContext = gpu::IGPUContext::create(m_window);
		// m_swapchain  = new gpu::VKSwapChain(m_gpuContext, m_window);

		BOOL useDarkMode = TRUE;
		DwmSetWindowAttribute(glfwGetWin32Window(m_window), DWMWA_USE_IMMERSIVE_DARK_MODE, &useDarkMode, sizeof(useDarkMode));

		glfwSetWindowUserPointer(m_window, &m_callbackData);

		glfwSetWindowSizeCallback(m_window, [](GLFWwindow *window, const int width, const int height)
		{
			auto &data = *(static_cast<GLFWCallbackData *>(glfwGetWindowUserPointer(window)));

			WindowResizeEvent event(width, height);
			if (data.eventCallback)
				data.eventCallback(event);
			data.width  = width;
			data.height = height;
		});

		glfwSetWindowCloseCallback(m_window, [](GLFWwindow *window)
		{
			const auto &data = *(static_cast<GLFWCallbackData *>(glfwGetWindowUserPointer(window)));

			WindowCloseEvent event;
			if (data.eventCallback)
				data.eventCallback(event);
		});

		glfwSetKeyCallback(m_window, [](GLFWwindow *window, int key, int scancode, int action, int mods)
		{
			const auto &data = *(static_cast<GLFWCallbackData *>(glfwGetWindowUserPointer(window)));

			switch (action)
			{
				case GLFW_PRESS:
				{
					KeyPressEvent event(static_cast<input::EKeyCode>(key), 0);
					if (data.eventCallback)
						data.eventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleaseEvent event(static_cast<input::EKeyCode>(key));
					if (data.eventCallback)
						data.eventCallback(event);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressEvent event(static_cast<input::EKeyCode>(key), 1);
					if (data.eventCallback)
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

			KeyTypeEvent event(static_cast<input::EKeyCode>(codepoint));
			if (data.eventCallback)
				data.eventCallback(event);
		});

		glfwSetMouseButtonCallback(m_window, [](GLFWwindow *window, int button, int action, int mods)
		{
			const auto &data = *(static_cast<GLFWCallbackData *>(glfwGetWindowUserPointer(window)));

			switch (action)
			{
				case GLFW_PRESS:
				{
					MouseButtonPressEvent event(static_cast<input::EMouseButton>(button));
					if (data.eventCallback)
						data.eventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleaseEvent event(static_cast<input::EMouseButton>(button));
					if (data.eventCallback)
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
			if (data.eventCallback)
				data.eventCallback(event);
		});

		glfwSetCursorPosCallback(m_window, [](GLFWwindow *window, double x, double y)
		{
			const auto &   data = *(static_cast<GLFWCallbackData *>(glfwGetWindowUserPointer(window)));
			MouseMoveEvent event(static_cast<float>(x), static_cast<float>(y));
			if (data.eventCallback)
				data.eventCallback(event);
		});

		glfwSetWindowMaximizeCallback(m_window, [](GLFWwindow *window, int maximized)
		{
			const auto &data = *(static_cast<GLFWCallbackData *>(glfwGetWindowUserPointer(window)));

			WindowMaximizeEvent event(static_cast<bool>(maximized));
			if (data.eventCallback)
				data.eventCallback(event);
		});

		glfwSetWindowIconifyCallback(m_window, [](GLFWwindow *window, int iconified)
		{
			const auto &data = *(static_cast<GLFWCallbackData *>(glfwGetWindowUserPointer(window)));

			WindowMinimizeEvent event(static_cast<bool>(iconified));
			if (data.eventCallback)
				data.eventCallback(event);
		});

		if (!p_create_info.iconPath.empty())
		{
			GLFWimage window_icon[1];

			int32 nr_channels;
			window_icon[0].pixels = stbi_load(p_create_info.iconPath.string().c_str(), &window_icon[0].width, &window_icon[0].height, &nr_channels, 4);
			if (window_icon[0].pixels)
			{
				glfwSetWindowIcon(m_window, 1, window_icon);
				stbi_image_free(window_icon[0].pixels);
			}
			else
			{
				LOG_ERROR("Failed to load image icon. Path: {}", p_create_info.iconPath.string());
			}
		}

		if (p_create_info.startMaximized)
			maximize();

		showWindow();
	}

	Window::~Window()
	{
		delete m_gpuContext;

		glfwDestroyWindow(m_window);
	}

	void Window::beginFrame()
	{
	}

	void Window::processEvents()
	{
		glfwPollEvents();
	}

	void Window::endFrame()
	{
		// glfwSwapBuffers(m_window);
	}

	void Window::showWindow()
	{
		glfwShowWindow(m_window);
	}

	void Window::hideWindow()
	{
		glfwHideWindow(m_window);
	}

	void Window::maximize()
	{
		glfwMaximizeWindow(m_window);
	}

	void Window::minimize()
	{
		glfwIconifyWindow(m_window);
	}

	void Window::restore()
	{
		glfwRestoreWindow(m_window);
	}

	void Window::fullscreen()
	{
		GLFWmonitor *      monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode *mode    = glfwGetVideoMode(monitor);

		glfwSetWindowMonitor(m_window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
	}

	void Window::setEventCallback(const EventCallbackFn &p_callback)
	{
		m_callbackData.eventCallback = p_callback;
	}

	uint32 Window::getWidth() const
	{
		return m_callbackData.width;
	}

	float32 Window::getAspect() const
	{
		return static_cast<float32>(m_callbackData.height) / static_cast<float32>(m_callbackData.width);
	}

	ScreenPos Window::getCenter() const
	{
		return {static_cast<float32>(m_callbackData.width) / 2.0f, static_cast<float32>(m_callbackData.height) / 2.0f};
	}

	uint32 Window::getHeight() const
	{
		return m_callbackData.height;
	}

	const std::string &Window::getTitle() const
	{
		return m_callbackData.title;
	}

	void Window::setTitle(const std::string &p_title)
	{
		m_callbackData.title = p_title;
		glfwSetWindowTitle(m_window, p_title.c_str());
	}

	gpu::IGPUContext *Window::getGPUContext() const
	{
		return m_gpuContext;
	}

	GLFWwindow *Window::getNativeWindow() const
	{
		return m_window;
	}
}
