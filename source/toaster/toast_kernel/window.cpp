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

#include "toast_lib/logging.hpp"
#include "toast_lib/toast_assert.h"

#include "toast_lib/events/key_event.hpp"
#include "toast_lib/events/mouse_event.hpp"
#include "toast_lib/events/window_event.hpp"

#include "toast_gpu/vk/vk_gpu_context.hpp"
#include "toast_gpu/vk/vk_swapchain.hpp"

namespace toaster
{
	static bool s_glfwInitialized{false};

	static auto _glfwErrorCallback(int32 error, CString description) -> void
	{
		LOG_ERROR("GLFW error: ({}): {}", error, description);
	}

	auto Window::initWindowingAPI() -> void
	{
		if (!s_glfwInitialized)
		{
			const bool init_result{static_cast<bool>(glfwInit())};
			TST_ASSERT_MSG(init_result, "glfw initialization failed!");

			glfwSetErrorCallback(_glfwErrorCallback);

			s_glfwInitialized = true;
		}
	}

	auto Window::shutdownWindowingAPI() -> void
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
		m_swapchain  = new gpu::VKSwapchain(dynamic_cast<gpu::VKGPUContext *>(m_gpuContext), m_window);

		constexpr BOOL use_dark_mode{TRUE};
		(void) DwmSetWindowAttribute(glfwGetWin32Window(m_window), DWMWA_USE_IMMERSIVE_DARK_MODE, &use_dark_mode, sizeof(use_dark_mode));

		glfwSetWindowUserPointer(m_window, &m_callbackData);

		#define GET_CB_DATA() *(static_cast<GLFWCallbackData *>(glfwGetWindowUserPointer(window)))
		glfwSetWindowSizeCallback(m_window, [](GLFWwindow *window, const int32 width, const int32 height)
		{
			auto &data{GET_CB_DATA()};

			WindowResizeEvent event{static_cast<uint32>(width), static_cast<uint32>(height)};
			if (data.eventCallback)
				data.eventCallback(event);
			data.width  = width;
			data.height = height;
		});

		glfwSetWindowCloseCallback(m_window, [](GLFWwindow *window)
		{
			const auto &data{GET_CB_DATA()};

			WindowCloseEvent event{};
			if (data.eventCallback)
				data.eventCallback(event);
		});

		glfwSetKeyCallback(m_window, [](GLFWwindow *window, int32 key, [[maybe_unused]] int32 scancode, const int32 action, [[maybe_unused]] int32 mods)
		{
			const auto &data{GET_CB_DATA()};

			switch (action)
			{
				case GLFW_PRESS:
				{
					KeyPressEvent event{static_cast<input::EKeyCode>(key), 0};
					if (data.eventCallback)
						data.eventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleaseEvent event{static_cast<input::EKeyCode>(key)};
					if (data.eventCallback)
						data.eventCallback(event);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressEvent event{static_cast<input::EKeyCode>(key), 1};
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
			const auto &data{GET_CB_DATA()};

			KeyTypeEvent event{static_cast<input::EKeyCode>(codepoint)};
			if (data.eventCallback)
				data.eventCallback(event);
		});

		glfwSetMouseButtonCallback(m_window, [](GLFWwindow *window, int32 button, int32 action, [[maybe_unused]] int32 mods)
		{
			const auto &data{GET_CB_DATA()};

			switch (action)
			{
				case GLFW_PRESS:
				{
					MouseButtonPressEvent event{static_cast<input::EMouseButton>(button)};
					if (data.eventCallback)
						data.eventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleaseEvent event{static_cast<input::EMouseButton>(button)};
					if (data.eventCallback)
						data.eventCallback(event);
					break;
				}
				default: break;
			}
		});

		glfwSetScrollCallback(m_window, [](GLFWwindow *window, float64 xOffset, float64 yOffset)
		{
			const auto &data{GET_CB_DATA()};

			MouseScrollEvent event{static_cast<float32>(xOffset), static_cast<float32>(yOffset)};
			if (data.eventCallback)
				data.eventCallback(event);
		});

		glfwSetCursorPosCallback(m_window, [](GLFWwindow *window, float64 x, float64 y)
		{
			const auto &data{GET_CB_DATA()};

			MouseMoveEvent event{static_cast<float32>(x), static_cast<float32>(y)};
			if (data.eventCallback)
				data.eventCallback(event);
		});

		glfwSetWindowMaximizeCallback(m_window, [](GLFWwindow *window, int32 maximized)
		{
			const auto &data{GET_CB_DATA()};

			WindowMaximizeEvent event{static_cast<bool>(maximized)};
			if (data.eventCallback)
				data.eventCallback(event);
		});

		glfwSetWindowIconifyCallback(m_window, [](GLFWwindow *window, int32 iconified)
		{
			const auto &data{GET_CB_DATA()};

			WindowMinimizeEvent event{static_cast<bool>(iconified)};
			if (data.eventCallback)
				data.eventCallback(event);
		});
		#undef GET_CB_DATA

		if (!p_create_info.iconPath.empty())
		{
			GLFWimage window_icon[1]{};

			int32 nr_channels{};
			window_icon[0].pixels = stbi_load(p_create_info.iconPath.string().c_str(), &window_icon[0].width, &window_icon[0].height, &nr_channels, 4);
			if (window_icon[0].pixels)
			{
				glfwSetWindowIcon(m_window, 1, window_icon);
				stbi_image_free(window_icon[0].pixels);
			}
			else
				LOG_ERROR("Failed to load image icon. Path: {}", p_create_info.iconPath.string());
		}

		if (p_create_info.startMaximized)
			maximize();

		showWindow();
	}

	Window::~Window()
	{
		dynamic_cast<gpu::VKGPUContext *>(m_gpuContext)->getDevice().waitIdle();
		delete m_swapchain;
		delete m_gpuContext;

		glfwDestroyWindow(m_window);
	}

	auto Window::beginFrame() -> void
	{
		m_swapchain->beginFrame();
	}

	auto Window::processEvents() -> void
	{
		glfwPollEvents();
	}

	auto Window::endFrame() -> void
	{
		m_swapchain->endFrame();
	}

	auto Window::showWindow() -> void
	{
		glfwShowWindow(m_window);
	}

	auto Window::hideWindow() -> void
	{
		glfwHideWindow(m_window);
	}

	auto Window::maximize() -> void
	{
		glfwMaximizeWindow(m_window);
	}

	auto Window::minimize() -> void
	{
		glfwIconifyWindow(m_window);
	}

	auto Window::restore() -> void
	{
		glfwRestoreWindow(m_window);
	}

	auto Window::fullscreen() -> void
	{
		GLFWmonitor *      monitor{glfwGetPrimaryMonitor()};
		const GLFWvidmode *mode{glfwGetVideoMode(monitor)};
		glfwSetWindowMonitor(m_window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
	}

	auto Window::setEventCallback(const EventCallbackFn &p_callback) -> void
	{
		m_callbackData.eventCallback = p_callback;
	}

	auto Window::getWidth() const -> uint32
	{
		return m_callbackData.width;
	}

	auto Window::getAspect() const -> float32
	{
		return static_cast<float32>(m_callbackData.height) / static_cast<float32>(m_callbackData.width);
	}

	auto Window::getCenter() const -> std::pair<float32, float32>
	{
		return {static_cast<float32>(m_callbackData.width) / 2.0f, static_cast<float32>(m_callbackData.height) / 2.0f};
	}

	auto Window::getHeight() const -> uint32
	{
		return m_callbackData.height;
	}

	auto Window::getTitle() const -> const std::string &
	{
		return m_callbackData.title;
	}

	auto Window::setTitle(const std::string &p_title) -> void
	{
		m_callbackData.title = p_title;
		glfwSetWindowTitle(m_window, p_title.c_str());
	}

	auto Window::getGPUContext() const -> gpu::IGPUContext *
	{
		return m_gpuContext;
	}

	auto Window::getNativeWindow() const -> GLFWwindow *
	{
		return m_window;
	}

	auto Window::getSwapchain() const -> gpu::VKSwapchain *
	{
		return m_swapchain;
	}
}
