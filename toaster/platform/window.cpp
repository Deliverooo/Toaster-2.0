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

#include "imgui.h"
#include "core/log.hpp"
#include "core/allocator.hpp"

#include "core/events/window_event.hpp"
#include "events/key_event.hpp"
#include "events/mouse_event.hpp"

namespace tst
{
	static const struct
	{
		nvrhi::Format format;
		uint32_t      redBits;
		uint32_t      greenBits;
		uint32_t      blueBits;
		uint32_t      alphaBits;
		uint32_t      depthBits;
		uint32_t      stencilBits;
	} formatInfo[] = {
		{nvrhi::Format::UNKNOWN, 0, 0, 0, 0, 0, 0,}, {nvrhi::Format::R8_UINT, 8, 0, 0, 0, 0, 0,}, {nvrhi::Format::RG8_UINT, 8, 8, 0, 0, 0, 0,},
		{nvrhi::Format::RG8_UNORM, 8, 8, 0, 0, 0, 0,}, {nvrhi::Format::R16_UINT, 16, 0, 0, 0, 0, 0,}, {nvrhi::Format::R16_UNORM, 16, 0, 0, 0, 0, 0,},
		{nvrhi::Format::R16_FLOAT, 16, 0, 0, 0, 0, 0,}, {nvrhi::Format::RGBA8_UNORM, 8, 8, 8, 8, 0, 0,}, {nvrhi::Format::RGBA8_SNORM, 8, 8, 8, 8, 0, 0,},
		{nvrhi::Format::BGRA8_UNORM, 8, 8, 8, 8, 0, 0,}, {nvrhi::Format::SRGBA8_UNORM, 8, 8, 8, 8, 0, 0,}, {nvrhi::Format::SBGRA8_UNORM, 8, 8, 8, 8, 0, 0,},
		{nvrhi::Format::R10G10B10A2_UNORM, 10, 10, 10, 2, 0, 0,}, {nvrhi::Format::R11G11B10_FLOAT, 11, 11, 10, 0, 0, 0,}, {nvrhi::Format::RG16_UINT, 16, 16, 0, 0, 0, 0,},
		{nvrhi::Format::RG16_FLOAT, 16, 16, 0, 0, 0, 0,}, {nvrhi::Format::R32_UINT, 32, 0, 0, 0, 0, 0,}, {nvrhi::Format::R32_FLOAT, 32, 0, 0, 0, 0, 0,},
		{nvrhi::Format::RGBA16_FLOAT, 16, 16, 16, 16, 0, 0,}, {nvrhi::Format::RGBA16_UNORM, 16, 16, 16, 16, 0, 0,}, {nvrhi::Format::RGBA16_SNORM, 16, 16, 16, 16, 0, 0,},
		{nvrhi::Format::RG32_UINT, 32, 32, 0, 0, 0, 0,}, {nvrhi::Format::RG32_FLOAT, 32, 32, 0, 0, 0, 0,}, {nvrhi::Format::RGB32_UINT, 32, 32, 32, 0, 0, 0,},
		{nvrhi::Format::RGB32_FLOAT, 32, 32, 32, 0, 0, 0,}, {nvrhi::Format::RGBA32_UINT, 32, 32, 32, 32, 0, 0,}, {nvrhi::Format::RGBA32_FLOAT, 32, 32, 32, 32, 0, 0,},
	};

	static void GLFWErrorCallback(int error, const char *description)
	{
		TST_ERROR_TAG("GLFW", "GLFW Error ({0}): {1}", error, description);
	}

	static bool s_GlfwInitialized = false;

	Window::Window(const WindowSpecInfo &window_spec_info) : m_specInfo(window_spec_info)
	{
	}

	Window::~Window()
	{
		terminate();
	}

	void Window::init()
	{
		m_callbackData.cbWidth  = m_specInfo.width;
		m_callbackData.cbHeight = m_specInfo.height;
		m_callbackData.cbTitle  = m_specInfo.title;

		DeviceCreationParameters deviceParams;
		deviceParams.swapChainBufferCount                    = 3;
		deviceParams.enableRayTracingExtensions              = true;
		deviceParams.maxFramesInFlight                       = 1;
		deviceParams.backBufferWidth                         = m_specInfo.width;
		deviceParams.backBufferHeight                        = m_specInfo.height;
		deviceParams.vsyncEnabled                            = false;
		deviceParams.enableDebugRuntime                      = true;
		deviceParams.ignoredVulkanValidationMessageLocations = {0xc81ad50e};

		if (!s_GlfwInitialized)
		{
			bool success = glfwInit();
			TST_ASSERT(success);

			glfwSetErrorCallback(GLFWErrorCallback);

			s_GlfwInitialized = true;
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		glfwDefaultWindowHints();

		bool foundFormat = false;
		for (const auto &info: formatInfo)
		{
			if (info.format == deviceParams.swapChainFormat)
			{
				glfwWindowHint(GLFW_RED_BITS, info.redBits);
				glfwWindowHint(GLFW_GREEN_BITS, info.greenBits);
				glfwWindowHint(GLFW_BLUE_BITS, info.blueBits);
				glfwWindowHint(GLFW_ALPHA_BITS, info.alphaBits);
				glfwWindowHint(GLFW_DEPTH_BITS, info.depthBits);
				glfwWindowHint(GLFW_STENCIL_BITS, info.stencilBits);
				foundFormat = true;
				break;
			}
		}

		TST_ASSERT(foundFormat);

		glfwWindowHint(GLFW_SAMPLES, deviceParams.swapChainSampleCount);
		glfwWindowHint(GLFW_REFRESH_RATE, deviceParams.refreshRate);

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		// glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // Ignored for fullscreen

		m_windowHandle = glfwCreateWindow(m_specInfo.width, m_specInfo.height, m_callbackData.cbTitle.c_str(), m_specInfo.fullscreen ? glfwGetPrimaryMonitor() : nullptr,
										  nullptr);

		glfwSetWindowUserPointer(m_windowHandle, this);

		if (m_specInfo.fullscreen)
		{
			glfwSetWindowMonitor(m_windowHandle, glfwGetPrimaryMonitor(), 0, 0, m_specInfo.width, m_specInfo.height, deviceParams.refreshRate);
		}
		else
		{
			int32 fbWidth  = 0;
			int32 fbHeight = 0;
			glfwGetFramebufferSize(m_windowHandle, &fbWidth, &fbHeight);
			m_callbackData.cbWidth  = fbWidth;
			m_callbackData.cbHeight = fbHeight;
		}

		if (deviceParams.windowPosX != -1 && deviceParams.windowPosY != -1)
		{
			glfwSetWindowPos(m_windowHandle, deviceParams.windowPosX, deviceParams.windowPosY);
		}

		nvrhi::GraphicsAPI api = nvrhi::GraphicsAPI::VULKAN;

		m_deviceManager = DeviceManager::Create(api, m_windowHandle);
		m_deviceManager->SetWindowContext(this);

		if (!m_deviceManager->CreateDevice(deviceParams, m_specInfo.title.c_str()))
		{
			TST_ERROR("Cannot initialize a {} graphics device.", (uint8_t)api);
			return;
		}
		else
		{
			TST_INFO("Successfully created {} device!", (uint8_t)api);
		}
		bool rayQuerySupported = m_deviceManager->GetDevice()->queryFeatureSupport(nvrhi::Feature::RayQuery);

		if (!rayQuerySupported)
		{
			TST_ERROR("The GPU ({}) or its driver does not support Ray Queries.", m_deviceManager->GetRendererString());
			return;
		}
		else
		{
			TST_INFO("rayQuerySupported=true");
		}

		if (!deviceParams.headlessDevice)
			_createWindowSurface();

		m_deviceManager->InitSurfaceCapabilities(*(uint64_t *) &m_windowSurface);

		m_vulkanSwapchain = tnew VulkanSwapChain(m_windowSurface);
		m_vulkanSwapchain->create(m_specInfo.width, m_specInfo.height);

		//glfwMaximizeWindow(m_Window);
		glfwSetWindowUserPointer(m_windowHandle, &m_callbackData);

		bool isRawMouseMotionSupported = glfwRawMouseMotionSupported();
		if (isRawMouseMotionSupported)
			glfwSetInputMode(m_windowHandle, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		else
			TST_WARN_TAG("Platform", "Raw mouse motion not supported.");

		_setGLFWCallbacks();

		m_ImGuiMouseCursors[ImGuiMouseCursor_Arrow]      = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
		m_ImGuiMouseCursors[ImGuiMouseCursor_TextInput]  = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
		m_ImGuiMouseCursors[ImGuiMouseCursor_ResizeAll]  = glfwCreateStandardCursor(GLFW_ARROW_CURSOR); // FIXME: GLFW doesn't have this.
		m_ImGuiMouseCursors[ImGuiMouseCursor_ResizeNS]   = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
		m_ImGuiMouseCursors[ImGuiMouseCursor_ResizeEW]   = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
		m_ImGuiMouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR); // FIXME: GLFW doesn't have this.
		m_ImGuiMouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR); // FIXME: GLFW doesn't have this.
		m_ImGuiMouseCursors[ImGuiMouseCursor_Hand]       = glfwCreateStandardCursor(GLFW_HAND_CURSOR);

		// Update window size to actual size
		{
			int width, height;
			glfwGetWindowSize(m_windowHandle, &width, &height);
			m_callbackData.cbWidth  = width;
			m_callbackData.cbHeight = height;
		}
	}

	void Window::terminate()
	{
		if (m_windowSurface)
		{
			auto vInstance = ((VulkanDeviceManager *) m_deviceManager)->GetVulkanInstance();
			TST_ASSERT(vInstance);
			vInstance.destroySurfaceKHR(m_windowSurface);
			m_windowSurface = nullptr;
		}

		m_deviceManager->Shutdown();
		tdelete m_deviceManager;

		s_GlfwInitialized = false;
	}

	void Window::processEvents()
	{
		glfwPollEvents();
		// Input::Update();

		// m_DeviceManager->UpdateWindowSize();
		int width;
		int height;
		glfwGetWindowSize(m_windowHandle, &width, &height);

		if (m_callbackData.cbWidth != width || m_callbackData.cbHeight != height)
		{
			m_callbackData.cbWidth  = width;
			m_callbackData.cbHeight = height;

			m_vulkanSwapchain->onResize(width, height);
		}
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

	tsm::vec2ui Window::getSize() const
	{
		int32 x;
		int32 y;
		glfwGetWindowSize(m_windowHandle, &x, &y);
		return {(uint32) x, (uint32) y};
	}

	void Window::setWindowSize(const tsm::vec2ui &size)
	{
		glfwSetWindowSize(m_windowHandle, size.x, size.y);
	}

	void Window::setWindowMinSize(const tsm::vec2ui &min_size)
	{
		glfwSetWindowSizeLimits(m_windowHandle, min_size.x, min_size.y, GLFW_DONT_CARE, GLFW_DONT_CARE);
	}

	void Window::setWindowMaxSize(const tsm::vec2ui &max_size)
	{
		glfwSetWindowSizeLimits(m_windowHandle, GLFW_DONT_CARE, GLFW_DONT_CARE, max_size.x, max_size.y);
	}

	void Window::setResizable(bool resizable)
	{
		glfwSetWindowAttrib(m_windowHandle, GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);
	}

	tsm::vec2ui Window::getPosition() const
	{
		int32 x;
		int32 y;
		glfwGetWindowPos(m_windowHandle, &x, &y);
		return {(uint32) x, (uint32) y};
	}

	void Window::setPosition(const tsm::vec2ui &pos)
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

	VulkanSwapChain *Window::getSwapchain()
	{
		return m_vulkanSwapchain;
	}

	DeviceManager *Window::getDeviceManager()
	{
		return m_deviceManager;
	}

	#ifdef TST_PLATFORM_WINDOWS
	HWND Window::getHWND() const
	{
		return glfwGetWin32Window(m_windowHandle);
	}
	#endif

	void Window::_createWindowSurface()
	{
		const vk::Result res = static_cast<vk::Result>(glfwCreateWindowSurface(dynamic_cast<VulkanDeviceManager *>(m_deviceManager)->GetVulkanInstance(), m_windowHandle,
																			   nullptr, reinterpret_cast<VkSurfaceKHR *>(&m_windowSurface)));
		if (res != vk::Result::eSuccess)
		{
			TST_ERROR("Failed to create a GLFW window surface, error code = {}", vk::to_string(res));
		}
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
			auto &data = *((CallbackData *) glfwGetWindowUserPointer(window));

			KeyTypedEvent event((EKeyCode) codepoint);
			data.cbFunction(event);
		});
		glfwSetMouseButtonCallback(m_windowHandle, [](GLFWwindow *window, int button, int action, int mods)
		{
			auto &data = *((CallbackData *) glfwGetWindowUserPointer(window));

			switch (action)
			{
				case GLFW_PRESS:
				{
					// Input::UpdateButtonState((EMouseButton) button, EKeyState::ePressed);
					MouseButtonPressEvent event((EMouseButton) button);
					data.cbFunction(event);
					break;
				}
				case GLFW_RELEASE:
				{
					// Input::UpdateButtonState((EMouseButton) button, EKeyState::eReleased);
					MouseButtonReleaseEvent event((EMouseButton) button);
					data.cbFunction(event);
					break;
				}
			}
		});
		glfwSetScrollCallback(m_windowHandle, [](GLFWwindow *window, double xOffset, double yOffset)
		{
			auto &data = *((CallbackData *) glfwGetWindowUserPointer(window));

			MouseScrollEvent event((float) xOffset, (float) yOffset);
			data.cbFunction(event);
		});
		glfwSetCursorPosCallback(m_windowHandle, [](GLFWwindow *window, double x, double y)
		{
			auto &         data = *((CallbackData *) glfwGetWindowUserPointer(window));
			MouseMoveEvent event((float) x, (float) y);
			data.cbFunction(event);
		});
		glfwSetWindowIconifyCallback(m_windowHandle, [](GLFWwindow *window, int iconified)
		{
			auto &               data = *((CallbackData *) glfwGetWindowUserPointer(window));
			WindowMinimizedEvent event((bool) iconified);
			data.cbFunction(event);
		});
	}
}
