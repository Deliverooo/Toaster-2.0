#include "window.hpp"

#include "gpu/vulkan/vulkan_swapchain.hpp"
#include "gpu/vulkan/vulkan_device_manager.hpp"

#ifdef TST_PLATFORM_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#include <Windows.h>
#include <dwmapi.h>
#endif

#include <GLFW/glfw3native.h>

#include <nvrhi/nvrhi.h>
#include <nvrhi/vulkan.h>

#include "logging/log.hpp"
#include "memory/allocator.hpp"

#include "core/events/window_event.hpp"
#include "events/key_event.hpp"

namespace tst
{
	static bool s_GlfwInitialized = false;

	Window::Window(const WindowSpecInfo &window_spec_info) : m_specInfo(window_spec_info)
	{
	}

	Window::~Window()
	{
	}

	EError Window::init()
	{
		m_callbackData.cbWidth  = m_specInfo.width;
		m_callbackData.cbHeight = m_specInfo.height;
		m_callbackData.cbTitle  = m_specInfo.title;

		gpu::DeviceSpecInfo deviceSpec{};
		deviceSpec.maxFramesInFlight = 1;
		deviceSpec.vSyncEnabled      = false;

		if (!s_GlfwInitialized)
		{
			bool success = glfwInit();
			TST_ASSERT(success);

			// glfwSetErrorCallback()

			s_GlfwInitialized = true;
		}

		glfwDefaultWindowHints();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		glfwWindowHint(GLFW_SAMPLES, deviceSpec.swapChainSampleCount);
		glfwWindowHint(GLFW_REFRESH_RATE, deviceSpec.refreshRate);

		m_windowHandle = glfwCreateWindow(m_specInfo.width, m_specInfo.height, m_callbackData.cbTitle.c_str(), m_specInfo.fullscreen ? glfwGetPrimaryMonitor() : nullptr,
										  nullptr);

		if (m_specInfo.fullscreen)
		{
			glfwSetWindowMonitor(m_windowHandle, glfwGetPrimaryMonitor(), 0, 0, m_specInfo.width, m_specInfo.height, deviceSpec.refreshRate);
		}
		else
		{
			int32 fbWidth  = 0;
			int32 fbHeight = 0;
			glfwGetFramebufferSize(m_windowHandle, &fbWidth, &fbHeight);
			m_callbackData.cbWidth  = fbWidth;
			m_callbackData.cbHeight = fbHeight;
		}

		nvrhi::GraphicsAPI api = nvrhi::GraphicsAPI::VULKAN;

		m_deviceManager = gpu::DeviceManager::create(api);

		if (m_deviceManager->createDevice(deviceSpec) != EError::eOk)
		{
			TST_ERROR("Cannot initialize a {} graphics device.", static_cast<uint8_t>(api));
			return EError::eDeviceError;
		}
		TST_INFO("Successfully created {} device!", static_cast<uint8_t>(api));

		_createWindowSurface();

		m_vulkanSwapchain = tnew gpu::VulkanSwapChain(m_windowSurface);
		m_vulkanSwapchain->create(m_specInfo.width, m_specInfo.height);

		glfwSetWindowUserPointer(m_windowHandle, &m_callbackData);

		if (glfwRawMouseMotionSupported())
		{
			glfwSetInputMode(m_windowHandle, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		}
		else
		{
			TST_WARN_TAG("Platform", "Raw mouse motion not supported.");
		}

		_setGLFWCallbacks();

		return EError::eOk;
	}

	void Window::processEvents()
	{
		glfwPollEvents();
	}

	void Window::beginFrame()
	{
		m_vulkanSwapchain->beginFrame();
	}

	void Window::present()
	{
		m_vulkanSwapchain->present();
	}

	bool Window::getWindowFlag(EWindowFlags window_flag) const
	{
		return m_windowFlags & static_cast<uint32>(window_flag);
	}

	void Window::setWindowFlag(EWindowFlags window_flag, bool enable)
	{
		#ifndef TST_PLATFORM_WINDOWS
		if (enable)
		{
			m_windowFlags |= static_cast<uint32>(window_flag);
		}
		else
		{
			m_windowFlags &= ~static_cast<uint32>(window_flag);
		}
		#endif

		#ifdef TST_PLATFORM_WINDOWS
		switch (window_flag)
		{
			case EWindowFlags::eResizeDisabled:
			{
				if (enable)
				{
					m_windowFlags |= static_cast<uint32>(window_flag);
				}
				else
				{
					m_windowFlags &= ~static_cast<uint32>(window_flag);
				}
			}
			case EWindowFlags::eBorderless:
			{
				if (enable)
				{
					m_windowFlags |= static_cast<uint32>(window_flag);
				}
				else
				{
					m_windowFlags &= ~static_cast<uint32>(window_flag);
				}
			}
			case EWindowFlags::eAlwaysOnTop:
			{
				if (enable)
				{
					m_windowFlags |= static_cast<uint32>(window_flag);
				}
				else
				{
					m_windowFlags &= ~static_cast<uint32>(window_flag);
				}
			}
			case EWindowFlags::eTransparent:
			{
				if (enable)
				{
					m_windowFlags |= static_cast<uint32>(window_flag);

					DWM_BLURBEHIND bb;
					ZeroMemory(&bb, sizeof(bb));
					HRGN hRgn   = CreateRectRgn(0, 0, -1, -1);
					bb.dwFlags  = DWM_BB_ENABLE | DWM_BB_BLURREGION;
					bb.hRgnBlur = hRgn;
					bb.fEnable  = TRUE;
					DwmEnableBlurBehindWindow(getHWND(), &bb);
				}
				else
				{
					m_windowFlags &= ~static_cast<uint32>(window_flag);

					DWM_BLURBEHIND bb;
					ZeroMemory(&bb, sizeof(bb));
					HRGN hRgn   = CreateRectRgn(0, 0, -1, -1);
					bb.dwFlags  = DWM_BB_ENABLE | DWM_BB_BLURREGION;
					bb.hRgnBlur = hRgn;
					bb.fEnable  = FALSE;
					DwmEnableBlurBehindWindow(getHWND(), &bb);
				}
			}
			case EWindowFlags::eNoFocus:
			{
				if (enable)
				{
					m_windowFlags |= static_cast<uint32>(window_flag);
				}
				else
				{
					m_windowFlags &= ~static_cast<uint32>(window_flag);
				}
			}
			case EWindowFlags::eSharpCorners:
			{
				if (enable)
				{
					m_windowFlags |= static_cast<uint32>(window_flag);
					DWORD value = DWMWCP_DONOTROUND;
					::DwmSetWindowAttribute(getHWND(), DWMWA_WINDOW_CORNER_PREFERENCE, &value, sizeof(value));
				}
				else
				{
					m_windowFlags &= ~static_cast<uint32>(window_flag);
					DWORD value = DWMWCP_DEFAULT;
					::DwmSetWindowAttribute(getHWND(), DWMWA_WINDOW_CORNER_PREFERENCE, &value, sizeof(value));
				}
			}
			case EWindowFlags::eMinimizeDisabled:
			{
				if (enable)
				{
					m_windowFlags |= static_cast<uint32>(window_flag);
				}
				else
				{
					m_windowFlags &= ~static_cast<uint32>(window_flag);
				}
			}
			case EWindowFlags::eMaximizeDisabled:
			{
				if (enable)
				{
					m_windowFlags |= static_cast<uint32>(window_flag);
				}
				else
				{
					m_windowFlags &= ~static_cast<uint32>(window_flag);
				}
			}
		}
		#endif
	}

	bool Window::isVsyncEnabled() const
	{
		// return m_vulkanSwapchain->isVsyncEnabled();
		return false;
	}

	void Window::setVsyncMode(EVsyncMode vsync_mode)
	{
		#if 0
		switch (vsync_mode)
		{
			case EVsyncMode::eEnabled:
			{
				m_vulkanSwapchain->enableVsync();
			}
			case EVsyncMode::eDisabled:
			{
				m_vulkanSwapchain->disableVsync();
			}
			case EVsyncMode::eAdaptive:
			{
				m_vulkanSwapchain->enableVsync();
			}
		}
		#endif
	}

	uint32 Window::getWidth() const
	{
		return m_callbackData.cbWidth;
	}

	uint32 Window::getHeight() const
	{
		return m_callbackData.cbHeight;
	}

	Vec2i Window::getSize() const
	{
		int32 x;
		int32 y;
		glfwGetWindowSize(m_windowHandle, &x, &y);
		return {x, y};
	}

	void Window::setWindowSize(const Vec2i &size)
	{
		glfwSetWindowSize(m_windowHandle, size.x, size.y);
	}

	void Window::setWindowMinSize(const Vec2i &min_size)
	{
		glfwSetWindowSizeLimits(m_windowHandle, min_size.x, min_size.y, GLFW_DONT_CARE, GLFW_DONT_CARE);
	}

	void Window::setWindowMaxSize(const Vec2i &max_size)
	{
		glfwSetWindowSizeLimits(m_windowHandle, GLFW_DONT_CARE, GLFW_DONT_CARE, max_size.x, max_size.y);
	}

	Vec2i Window::getPosition() const
	{
		int32 x;
		int32 y;
		glfwGetWindowPos(m_windowHandle, &x, &y);
		return {x, y};
	}

	void Window::setPosition(const Vec2i &pos)
	{
		glfwSetWindowPos(m_windowHandle, pos.x, pos.y);
	}

	const String &Window::getTitle() const
	{
		return m_callbackData.cbTitle;
	}

	void Window::setTitle(const String &title)
	{
		m_callbackData.cbTitle = title;
		glfwSetWindowTitle(m_windowHandle, m_callbackData.cbTitle.c_str());
	}

	void Window::minimize()
	{
		#ifdef TST_PLATFORM_WINDOWS

		ShowWindow(getHWND(), SW_MINIMIZE);

		#endif
	}

	void Window::maximize()
	{
		glfwMaximizeWindow(m_windowHandle);
	}

	void Window::centerWindow()
	{
		const GLFWvidmode *vid_mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		int                x        = (vid_mode->width / 2) - (m_callbackData.cbWidth / 2);
		int                y        = (vid_mode->height / 2) - (m_callbackData.cbHeight / 2);
		glfwSetWindowPos(m_windowHandle, x, y);
	}

	GLFWwindow *Window::getNativeWindow() const
	{
		return m_windowHandle;
	}

	gpu::VulkanSwapChain *Window::getSwapchain()
	{
		return m_vulkanSwapchain;
	}

	gpu::DeviceManager *Window::getDeviceManager()
	{
		return m_deviceManager;
	}

	#ifdef TST_PLATFORM_WINDOWS
	HWND Window::getHWND() const
	{
		return glfwGetWin32Window(m_windowHandle);
	}
	#endif

	EError Window::_createWindowSurface()
	{
		const vk::Result res = static_cast<vk::Result>(glfwCreateWindowSurface(dynamic_cast<gpu::VulkanDeviceManager *>(m_deviceManager)->getVulkanInstance(),
																			   m_windowHandle, nullptr, reinterpret_cast<VkSurfaceKHR *>(&m_windowSurface)));
		if (res != vk::Result::eSuccess)
		{
			TST_ERROR("Failed to create a GLFW window surface, error code = {}", vk::to_string(res));
			return EError::eFailedToCreate;
		}

		return EError::eOk;
	}

	void Window::_setGLFWCallbacks()
	{
		glfwSetWindowSizeCallback(m_windowHandle, [](GLFWwindow *window, int width, int height)
		{
			const auto &data = static_cast<CallbackData *>(glfwGetWindowUserPointer(window));
			data->cbWidth    = width;
			data->cbHeight   = height;
		});
		glfwSetWindowCloseCallback(m_windowHandle, [](GLFWwindow *window)
		{
			const auto &data = *static_cast<CallbackData *>(glfwGetWindowUserPointer(window));

			WindowClosedEvent event;
			data.cbFunction(event);
		});
		glfwSetKeyCallback(m_windowHandle, [](GLFWwindow *window, int key, int scancode, int action, int mods)
		{
			const auto &data = *static_cast<CallbackData *>(glfwGetWindowUserPointer(window));

			switch (action)
			{
				case GLFW_PRESS:
				{
					// Input::UpdateKeyState((KeyCode) key, KeyState::Pressed);
					KeyPressedEvent event(static_cast<EKeyCode>(key), 0);
					data.cbFunction(event);
					break;
				}
				case GLFW_RELEASE:
				{
					// Input::UpdateKeyState((KeyCode) key, KeyState::Released);
					KeyReleasedEvent event(static_cast<EKeyCode>(key));
					data.cbFunction(event);
					break;
				}
				case GLFW_REPEAT:
				{
					// Input::UpdateKeyState((KeyCode) key, KeyState::Held);
					KeyPressedEvent event(static_cast<EKeyCode>(key), 1);
					data.cbFunction(event);
					break;
				}
			}
		});
		glfwSetCharCallback(m_windowHandle, [](GLFWwindow *window, uint32_t codepoint)
		{
		});
		glfwSetMouseButtonCallback(m_windowHandle, [](GLFWwindow *window, int button, int action, int mods)
		{
		});
		glfwSetScrollCallback(m_windowHandle, [](GLFWwindow *window, double xOffset, double yOffset)
		{
		});
		glfwSetCursorPosCallback(m_windowHandle, [](GLFWwindow *window, double x, double y)
		{
		});
		glfwSetWindowIconifyCallback(m_windowHandle, [](GLFWwindow *window, int iconified)
		{
		});
	}
}
