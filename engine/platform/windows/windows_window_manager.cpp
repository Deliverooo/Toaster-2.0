#include "windows_window_manager.hpp"

#include <avrt.h>
#include <ranges>
#include <shlwapi.h>
#include <shobjidl.h>
#include <ShellScalingApi.h>

#include "windows_os.hpp"

#include "gpu/vulkan/vulkan_rendering_context.hpp"
#include "logging/log.hpp"

#undef min
#undef max

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

	WindowsWindowManager::WindowsWindowManager(const String &  render_api_str, EWindowMode mode, EVsyncMode vsync_mode, uint32     flags, const Vector2I *position,
											   const Vector2I &resolution, int             screen, int64    parent_window, EError &return_error)
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
			m_renderingContext = tnew
			gpu::OpenGLRenderingContext();
		}
		#endif

		if (m_renderingContext)
		{
			if (m_renderingContext->initialize() != EError::eOk)
			{
				TST_LOG_CRITICAL("Failed to initialize rendering context");
				tdelete m_renderingContext;
			}
		}

		Vector2I window_position;
		if (position != nullptr)
		{
			window_position = *position;
		}
		else
		{
			if (screen == SCREEN_OF_MAIN_WINDOW)
			{
				screen = SCREEN_PRIMARY;
			}
			Rect2I scr_rect = screen_get_usable_rect(screen);
			window_position = scr_rect + (scr_rect.width - resolution) / 2;
		}

		HWND parent_hwnd = nullptr;
		if (parent_window)
		{
			// Parented window.
			parent_hwnd = (HWND) parent_window;
		}

		WindowID mainWindow = WindowsWindowManager::createWindow(mode, vsync_mode, flags, Rect2I(window_position, resolution), false, INVALID_WINDOW_ID, parent_hwnd);
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

	WindowID WindowsWindowManager::_createWindow(EWindowMode mode, EVsyncMode vsync_mode, uint32 flags, const Rect2I &rect, bool exclusive, HWND parent_hwnd)
	{
		DWORD dwExStyle;
		DWORD dwStyle;

		const bool main_window       = m_windowIdCounter == MAIN_WINDOW_ID;
		const bool embed_child       = parent_hwnd;
		const bool initialized       = true;
		const bool fullscreen        = (mode == EWindowMode::eFullscreen || mode == EWindowMode::eExclusiveFullscreen);
		const bool borderless        = (int) flags & (int) EWindowFlags::eBorderless;
		const bool resizable         = !((int) flags & (int) EWindowFlags::eResizeDisabled);
		const bool no_min_btn        = (int) flags & (int) EWindowFlags::eMinimizeDisabled;
		const bool no_max_btn        = (int) flags & (int) EWindowFlags::eMaximizeDisabled;
		const bool minimized         = mode == EWindowMode::eMinimized;
		const bool maximized         = mode == EWindowMode::eMaximized;
		const bool no_activate_focus = ((int) flags & (int) EWindowFlags::eNoFocus) | ((int) flags & (int) EWindowFlags::ePopup);

		getWindowStyle(main_window, embed_child, initialized, fullscreen, borderless, resizable, no_min_btn, no_max_btn, minimized, maximized, no_activate_focus, dwStyle,
					   dwExStyle);

		int rq_screen = getScreenFromRect(rect);
		if (rq_screen < 0) {
			rq_screen = getPrimaryScreen(); // Requested window rect is outside any screen bounds.
		}
		Rect2I usable_rect = getScreenUsableRect(rq_screen);

		Vector2I offset = _getScreensOrigin();

		RECT windowRect{};

		int off_x = (mode == EWindowMode::eFullscreen || (((int32) flags & (int32) EWindowFlags::eBorderless) && mode == EWindowMode::eMaximized)) ? 2 : 0;

		windowRect.left   = rect.x;
		windowRect.right  = rect.x + rect.width + off_x;
		windowRect.top    = rect.x;
		windowRect.bottom = rect.x + rect.height;

		if (!parent_hwnd)
		{
			if (mode == EWindowMode::eFullscreen || mode == EWindowMode::eExclusiveFullscreen)
			{
				Rect2I screen_rect = Rect2I(getScreenPosition(rq_screen), getScreenSize(rq_screen));

				windowRect.left   = screen_rect.position.x;
				windowRect.right  = screen_rect.position.x + screen_rect.resolution.x + off_x;
				windowRect.top    = screen_rect.position.y;
				windowRect.bottom = screen_rect.position.y + screen_rect.resolution.y;
			}
			else
			{
				Rect2I   srect = getScreenUsableRect(rq_screen);
				Vector2I wpos  = rect.position;
				if (srect != Rect2I())
				{
					wpos = wpos.clamp(srect.position, srect.position + srect.resolution - rect.resolution / 3);
				}

				windowRect.left   = wpos.x;
				windowRect.right  = wpos.x + rect.resolution.x + off_x;
				windowRect.top    = wpos.y;
				windowRect.bottom = wpos.y + rect.resolution.y;
			}
		}

		WindowID id = m_windowIdCounter;

		HWND owner_hwnd = nullptr;

		WindowData &wd = m_windows[id];
		wd.id          = id;
		wd.hWnd        = CreateWindowExW(dwExStyle, L"Toaster", L"", dwStyle, windowRect.left, windowRect.top, windowRect.right - windowRect.left,
										 windowRect.bottom - windowRect.top, owner_hwnd, nullptr, m_hInstance, &wd);
		if (!wd.hWnd)
		{
			MessageBoxW(nullptr, L"Window Creation Error.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
			m_windows.erase(id);

			TST_LOG_ERROR("Failed to create Windows window.");
			return INVALID_WINDOW_ID;
		}
		wd.parentWindow = parent_hwnd;

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

	struct EnumScreenData
	{
		int      count;
		int      screen;
		HMONITOR monitor;
	};

	static BOOL CALLBACK _MonitorEnumProcPrim(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
	{
		EnumScreenData *data = reinterpret_cast<EnumScreenData *>(dwData);
		if ((lprcMonitor->left == 0) && (lprcMonitor->top == 0))
		{
			data->screen = data->count;
			return FALSE;
		}

		data->count++;
		return TRUE;
	}

	static BOOL CALLBACK _MonitorEnumProcScreen(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
	{
		EnumScreenData *data = reinterpret_cast<EnumScreenData *>(dwData);
		if (data->monitor == hMonitor)
		{
			data->screen = data->count;
		}

		data->count++;
		return TRUE;
	}

	static BOOL CALLBACK _MonitorEnumProcCount(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
	{
		int *data = (int *) dwData;
		(*data)++;
		return TRUE;
	}

	int WindowsWindowManager::getScreenCount() const
	{
		int data = 0;
		EnumDisplayMonitors(nullptr, nullptr, _MonitorEnumProcCount, reinterpret_cast<LPARAM>(&data));
		return data;
	}

	int WindowsWindowManager::getPrimaryScreen() const
	{
		EnumScreenData data = {0, 0, nullptr};
		EnumDisplayMonitors(nullptr, nullptr, _MonitorEnumProcPrim, (LPARAM) &data);
		return data.screen;
	}

	int WindowsWindowManager::getKeyboardFocusScreen() const
	{
		HWND hwnd = GetForegroundWindow();
		if (hwnd)
		{
			EnumScreenData data = {0, 0, MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST)};
			EnumDisplayMonitors(nullptr, nullptr, _MonitorEnumProcScreen, (LPARAM) &data);
			return data.screen;
		}
		return getPrimaryScreen();
	}

	struct EnumPosData
	{
		int     count;
		int     screen;
		Vector2 pos;
	};

	static BOOL CALLBACK _MonitorEnumProcPos(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
	{
		EnumPosData *data = (EnumPosData *) dwData;
		if (data->count == data->screen)
		{
			data->pos.x = lprcMonitor->left;
			data->pos.y = lprcMonitor->top;
		}

		data->count++;
		return TRUE;
	}

	static BOOL CALLBACK _MonitorEnumProcOrigin(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
	{
		EnumPosData *data = (EnumPosData *) dwData;
		data->pos         = data->pos.min(Vector2(lprcMonitor->left, lprcMonitor->top));

		return TRUE;
	}

	Vector2I WindowsWindowManager::_getScreensOrigin() const
	{
		EnumPosData data = {0, 0, Vector2()};
		EnumDisplayMonitors(nullptr, nullptr, _MonitorEnumProcOrigin, (LPARAM) &data);
		return Vector2I{data.pos};
	}

	Vector2I WindowsWindowManager::getScreenPosition(int screen) const
	{
		screen           = _getScreenIndex(screen);
		int screen_count = getScreenCount();
		if (screen < 0 || screen >= screen_count)
		{
			return Vector2I();
		}

		EnumPosData data = {0, screen, Vector2()};
		EnumDisplayMonitors(nullptr, nullptr, _MonitorEnumProcPos, (LPARAM) &data);
		return data.pos - _getScreensOrigin();
	}

	struct EnumSizeData
	{
		int   count;
		int   screen;
		Size2 size;
	};

	struct EnumRectData
	{
		int    count;
		int    screen;
		Rect2I rect;
	};

	struct EnumRefreshRateData
	{
		std::vector<DISPLAYCONFIG_PATH_INFO> paths;
		std::vector<DISPLAYCONFIG_MODE_INFO> modes;
		int                                  count;
		int                                  screen;
		float                                rate;
	};

	static BOOL CALLBACK _MonitorEnumProcSize(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
	{
		EnumSizeData *data = (EnumSizeData *) dwData;
		if (data->count == data->screen)
		{
			data->size.x = lprcMonitor->right - lprcMonitor->left;
			data->size.y = lprcMonitor->bottom - lprcMonitor->top;
		}

		data->count++;
		return TRUE;
	}

	Size2I WindowsWindowManager::getScreenSize(int screen) const
	{
		screen           = _getScreenIndex(screen);
		int screen_count = getScreenCount();
		if (screen < 0 || screen >= screen_count)
		{
			return Size2I();
		}

		EnumSizeData data = {0, screen, Size2()};
		EnumDisplayMonitors(nullptr, nullptr, _MonitorEnumProcSize, (LPARAM) &data);
		return {(int) data.size.x, (int) data.size.y};
	}

	static BOOL CALLBACK _MonitorEnumProcUsableSize(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
	{
		EnumRectData *data = (EnumRectData *) dwData;
		if (data->count == data->screen)
		{
			MONITORINFO minfo;
			memset(&minfo, 0, sizeof(MONITORINFO));
			minfo.cbSize = sizeof(MONITORINFO);
			GetMonitorInfoA(hMonitor, &minfo);

			data->rect.x      = minfo.rcWork.left;
			data->rect.y      = minfo.rcWork.top;
			data->rect.width  = minfo.rcWork.right - minfo.rcWork.left;
			data->rect.height = minfo.rcWork.bottom - minfo.rcWork.top;
		}

		data->count++;
		return TRUE;
	}

	static BOOL CALLBACK _MonitorEnumProcRefreshRate(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
	{
		EnumRefreshRateData *data = (EnumRefreshRateData *) dwData;
		if (data->count == data->screen)
		{
			MONITORINFOEXW minfo;
			memset(&minfo, 0, sizeof(minfo));
			minfo.cbSize = sizeof(minfo);
			GetMonitorInfoW(hMonitor, &minfo);

			bool found = false;
			for (const DISPLAYCONFIG_PATH_INFO &path: data->paths)
			{
				DISPLAYCONFIG_SOURCE_DEVICE_NAME source_name;
				memset(&source_name, 0, sizeof(source_name));
				source_name.header.type      = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME;
				source_name.header.size      = sizeof(source_name);
				source_name.header.adapterId = path.sourceInfo.adapterId;
				source_name.header.id        = path.sourceInfo.id;
				if (DisplayConfigGetDeviceInfo(&source_name.header) == ERROR_SUCCESS)
				{
					if (wcscmp(minfo.szDevice, source_name.viewGdiDeviceName) == 0 && path.targetInfo.refreshRate.Numerator != 0 && path.targetInfo.refreshRate.
						Denominator != 0)
					{
						data->rate = static_cast<double>(path.targetInfo.refreshRate.Numerator) / static_cast<double>(path.targetInfo.refreshRate.Denominator);
						found      = true;
						break;
					}
				}
			}
			if (!found)
			{
				DEVMODEW dm;
				memset(&dm, 0, sizeof(dm));
				dm.dmSize = sizeof(dm);
				EnumDisplaySettingsW(minfo.szDevice, ENUM_CURRENT_SETTINGS, &dm);

				data->rate = dm.dmDisplayFrequency;
			}
		}

		data->count++;
		return TRUE;
	}

	Rect2I WindowsWindowManager::getScreenUsableRect(int screen) const
	{
		screen           = _getScreenIndex(screen);
		int screen_count = getScreenCount();
		if (screen < 0 || screen >= screen_count)
		{
			return Rect2I();
		}

		EnumRectData data = {0, screen, Rect2I()};
		EnumDisplayMonitors(nullptr, nullptr, _MonitorEnumProcUsableSize, (LPARAM) &data);
		data.rect.position -= _getScreensOrigin();
		return data.rect;
	}

	struct EnumDpiData
	{
		int count;
		int screen;
		int dpi;
	};

	static int QueryDpiForMonitor(HMONITOR hmon, MONITOR_DPI_TYPE dpiType = MDT_DEFAULT)
	{
		int dpiX = 96, dpiY = 96;

		UINT x = 0, y = 0;
		if (hmon)
		{
			HRESULT hr = GetDpiForMonitor(hmon, dpiType, &x, &y);
			if (SUCCEEDED(hr) && (x > 0) && (y > 0))
			{
				dpiX = (int) x;
				dpiY = (int) y;
			}
		}
		else
		{
			static int overallX = 0, overallY = 0;
			if (overallX <= 0 || overallY <= 0)
			{
				HDC hdc = GetDC(nullptr);
				if (hdc)
				{
					overallX = GetDeviceCaps(hdc, LOGPIXELSX);
					overallY = GetDeviceCaps(hdc, LOGPIXELSY);
					ReleaseDC(nullptr, hdc);
				}
			}
			if (overallX > 0 && overallY > 0)
			{
				dpiX = overallX;
				dpiY = overallY;
			}
		}

		return (dpiX + dpiY) / 2;
	}

	static BOOL CALLBACK _MonitorEnumProcDpi(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
	{
		EnumDpiData *data = (EnumDpiData *) dwData;
		if (data->count == data->screen)
		{
			data->dpi = QueryDpiForMonitor(hMonitor);
		}

		data->count++;
		return TRUE;
	}

	int WindowsWindowManager::getScreenDPI(int screen) const
	{
		screen           = _getScreenIndex(screen);
		int screen_count = getScreenCount();
		if (screen < 0 || screen >= screen_count)
		{
			return 72;
		}

		EnumDpiData data = {0, screen, 72};
		EnumDisplayMonitors(nullptr, nullptr, _MonitorEnumProcDpi, (LPARAM) &data);
		return data.dpi;
	}

	float WindowsWindowManager::getScreenRefreshRate(int screen) const
	{
		screen           = _getScreenIndex(screen);
		int screen_count = getScreenCount();
		if (screen < 0 || screen >= screen_count)
		{
			return -1.0;
		}

		EnumRefreshRateData data = {std::vector<DISPLAYCONFIG_PATH_INFO>(), std::vector<DISPLAYCONFIG_MODE_INFO>(), 0, screen, -1.0};

		uint32_t path_count = 0;
		uint32_t mode_count = 0;
		if (GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &path_count, &mode_count) == ERROR_SUCCESS)
		{
			data.paths.resize(path_count);
			data.modes.resize(mode_count);
			if (QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &path_count, data.paths.data(), &mode_count, data.modes.data(), nullptr) != ERROR_SUCCESS)
			{
				data.paths.clear();
				data.modes.clear();
			}
		}

		EnumDisplayMonitors(nullptr, nullptr, _MonitorEnumProcRefreshRate, (LPARAM) &data);
		return data.rate;
	}
}
