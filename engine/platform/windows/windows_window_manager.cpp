#include "windows_window_manager.hpp"

#include <avrt.h>
#include <ranges>
#include <shlwapi.h>
#include <shobjidl.h>

#include "windows_os.hpp"


#include "gpu/vulkan/vulkan_rendering_context.hpp"

namespace tst
{
	LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (auto *wm = dynamic_cast<WindowsWindowManager *>(WindowManager::get()))
		{
			return wm->WndProc(hWnd, uMsg, wParam, lParam);
		}
		return DefWindowProcW(hWnd, uMsg, wParam, lParam);
	}

	WindowsWindowManager::WindowsWindowManager()
	{
		m_hInstance = dynamic_cast<WindowsOS *>(OS::get())->getInstanceHandle();

		OleInitialize(nullptr);
		memset(&m_windowClass, 0, sizeof(WNDCLASSEXW));
		m_windowClass.cbSize        = sizeof(WNDCLASSEXW);
		m_windowClass.style         = CS_OWNDC | CS_DBLCLKS;
		m_windowClass.lpfnWndProc   = static_cast<WNDPROC>(tst::WndProc);
		m_windowClass.cbClsExtra    = 0;
		m_windowClass.cbWndExtra    = 0;
		m_windowClass.hInstance     = m_hInstance ? m_hInstance : GetModuleHandle(nullptr);
		m_windowClass.hIcon         = LoadIcon(nullptr, IDI_WINLOGO);
		m_windowClass.hCursor       = nullptr;
		m_windowClass.hbrBackground = nullptr;
		m_windowClass.lpszMenuName  = nullptr;
		m_windowClass.lpszClassName = L"Toaster";

		if (!RegisterClassExW(&m_windowClass))
		{
			TST_ASSERT(false);
			return;
		}

		#if defined(TST_ENABLE_VULKAN)
		if (m_graphicsAPI == "Vulkan")
		{
			m_renderingContext = tnew gpu::VulkanRenderingContext();
		}

		#elif defined(TST_ENABLE_OPENGL)
		if (m_graphicsAPI == "OpenGL")
		{
			m_renderingContext = tnew gpu::OpenGLRenderingContext();
		}
		#endif

		WindowID mainWindow = WindowsWindowManager::createWindow(EWindowMode::eWindowed, Rect2I(500, 500, 500, 500), EWindowFlags::eMaximizeDisabled,
																 EVsyncMode::eEnabled, INVALID_WINDOW_ID);
	}

	WindowsWindowManager::~WindowsWindowManager()
	{
	}

	LRESULT WindowsWindowManager::WndProcFileDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	LRESULT WindowsWindowManager::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	LRESULT WindowsWindowManager::MouseProc(int code, WPARAM wParam, LPARAM lParam)
	{
		return ::CallNextHookEx(nullptr, code, wParam, lParam);
	}

	void WindowsWindowManager::getWindowStyle(bool main_window, bool embed_child, bool initialized, bool fullscreen, bool borderless, bool resizable, bool no_min_btn,
											  bool no_max_btn, bool  minimized, bool   maximized, bool   no_activate_focus, DWORD &out_style, DWORD &out_style_ex)
	{
		// Windows docs for window styles:
		// https://docs.microsoft.com/en-us/windows/win32/winmsg/window-styles
		// https://docs.microsoft.com/en-us/windows/win32/winmsg/extended-window-styles

		out_style    = 0;
		out_style_ex = WS_EX_WINDOWEDGE;
		if (main_window)
		{
			// When embedded, we don't want the window to have WS_EX_APPWINDOW because it will
			// show the embedded process in the taskbar and Alt-Tab.
			if (!embed_child)
			{
				out_style_ex |= WS_EX_APPWINDOW;
			}
			if (initialized)
			{
				out_style |= WS_VISIBLE;
			}
		}

		if (embed_child)
		{
			out_style |= WS_POPUP;
		}
		else if (fullscreen || borderless)
		{
			out_style |= WS_POPUP; // p_borderless was WS_EX_TOOLWINDOW in the past.
			if (minimized)
			{
				out_style |= WS_MINIMIZE;
			}
			else if (maximized)
			{
				out_style |= WS_MAXIMIZE;
			}
			if (!fullscreen)
			{
				out_style |= WS_SYSMENU;
				if (!no_min_btn)
				{
					out_style |= WS_MINIMIZEBOX;
				}
				if (!no_max_btn)
				{
					out_style |= WS_MAXIMIZEBOX;
				}
			}
		}
		else
		{
			if (resizable)
			{
				if (minimized)
				{
					out_style = WS_OVERLAPPEDWINDOW | WS_MINIMIZE;
				}
				else if (maximized)
				{
					out_style = WS_OVERLAPPEDWINDOW | WS_MAXIMIZE;
				}
				else
				{
					out_style = WS_OVERLAPPEDWINDOW;
				}
				if (no_min_btn)
				{
					out_style &= ~WS_MINIMIZEBOX;
				}
				if (no_max_btn)
				{
					out_style &= ~WS_MAXIMIZEBOX;
				}
			}
			else
			{
				if (minimized)
				{
					out_style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZE;
				}
				else
				{
					out_style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
				}
				if (!no_min_btn)
				{
					out_style |= WS_MINIMIZEBOX;
				}
				if (!no_max_btn)
				{
					out_style |= WS_MAXIMIZEBOX;
				}
			}
		}

		if (no_activate_focus && !embed_child)
		{
			out_style_ex |= WS_EX_TOPMOST | WS_EX_NOACTIVATE;
		}

		if (!borderless && !no_activate_focus && initialized)
		{
			out_style |= WS_VISIBLE;
		}

		out_style |= WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
		out_style_ex |= WS_EX_ACCEPTFILES;
	}

	WindowID WindowsWindowManager::createWindow(EWindowMode mode, const Rect2I &rect, EWindowFlags flags, EVsyncMode vsync_mode, WindowID parent_id)
	{
		DWORD dwExStyle;
		DWORD dwStyle;

		const bool main_window       = m_windowIdCounter == MAIN_WINDOW_ID;
		const bool embed_child       = parent_id != INVALID_WINDOW_ID;
		const bool initialized       = true;
		const bool fullscreen        = (mode == EWindowMode::eFullscreen || mode == EWindowMode::eBorderlessFullscreen);
		const bool borderless        = (int) flags & (int) EWindowFlags::eBorderless;
		const bool resizable         = !((int) flags & (int) EWindowFlags::eResizeDisabled);
		const bool no_min_btn        = (int) flags & (int) EWindowFlags::eMinimizeDisabled;
		const bool no_max_btn        = (int) flags & (int) EWindowFlags::eMaximizeDisabled;
		const bool minimized         = mode == EWindowMode::eMinimized;
		const bool maximized         = mode == EWindowMode::eMaximized;
		const bool no_activate_focus = ((int) flags & (int) EWindowFlags::eNoFocus) | ((int) flags & (int) EWindowFlags::ePopup);

		getWindowStyle(main_window, embed_child, initialized, fullscreen, borderless, resizable, no_min_btn, no_max_btn, minimized, maximized, no_activate_focus, dwStyle,
					   dwExStyle);

		RECT windowRect{};

		windowRect.left   = rect.x;
		windowRect.right  = rect.x + rect.width;
		windowRect.top    = rect.x;
		windowRect.bottom = rect.x + rect.height;

		WindowID id = m_windowIdCounter;

		HWND owner_hwnd = nullptr;

		WindowData &wd = m_windows[id];
		wd.id          = id;
		wd.hWnd        = CreateWindowExW(dwExStyle, L"Toaster", L"", dwStyle, windowRect.left, windowRect.top, windowRect.right - windowRect.left,
										 windowRect.bottom - windowRect.top, owner_hwnd, nullptr, m_hInstance, &wd);

		wd.parentWindow = nullptr;

		m_windowIdCounter++;

		return id;
	}

	void WindowsWindowManager::destroyWindow(WindowID window_id)
	{
		if (const auto it = m_windows.find(window_id); it != m_windows.end())
		{
			const WindowData &wd     = m_windows[window_id];
			HWND              handle = wd.hWnd;

			DestroyWindow(handle);
		}
	}

	void WindowsWindowManager::showWindow(WindowID window_id)
	{
		if (const auto it = m_windows.find(window_id); it != m_windows.end())
		{
			const WindowData &wd     = m_windows[window_id];
			HWND              handle = wd.hWnd;
			ShowWindow(handle, SW_SHOW);
		}
	}

	void WindowsWindowManager::hideWindow(WindowID window_id)
	{
		if (const auto it = m_windows.find(window_id); it != m_windows.end())
		{
			const WindowData &wd     = m_windows[window_id];
			HWND              handle = wd.hWnd;
			// ... todo
		}
	}

	void WindowsWindowManager::setWindowEventCallback(WindowID window_id, const WindowEventCallback &callback)
	{
		if (const auto it = m_windows.find(window_id); it != m_windows.end())
		{
			m_windows[window_id].eventCallback = callback;
		}
	}

	void WindowsWindowManager::setInputEventCallback(WindowID window_id, const InputEventCallback &callback)
	{
		if (const auto it = m_windows.find(window_id); it != m_windows.end())
		{
			m_windows[window_id].inputEventCallback = callback;
		}
	}

	void WindowsWindowManager::processEvents()
	{
		MSG msg = {};
		while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}

	void WindowsWindowManager::swapBuffers()
	{
	}
}
