#pragma once

#include "core/core.hpp"

namespace tst
{
	using WindowID                  = int;
	constexpr int INVALID_WINDOW_ID = -1;

	enum class EWindowFlags
	{
		eResizeDisabled,
		eBorderless,
		eAlwaysOnTop,
		eTransparent,
		eNoFocus,
		ePopup,
		eExtendToTitle,
		eMousePassthrough,
		eSharpCorners,
		eExcludeFromCapture,
		ePopupWmHint,
		eMinimizeDisabled,
		eMaximizeDisabled,
		eMax, };

	enum class EWindowMode
	{
		eWindowed,
		eFullscreen,
		eMaximized,
		eMinimized,
		eExclusiveFullscreen, };

	enum class EVsyncMode
	{
		eDisabled, eEnabled, eAdaptive
	};

	enum class EWindowEvent
	{
		eMouseEnter,
		eMouseExit,
		eFocusIn,
		eFocusOut,
		eCloseRequest,
		eGoBackRequest,
		eDPIChange,
		eTitleBarChange,
		eForceClose, };

	enum Screens
	{
		INVALID_SCREEN             = -1,
		SCREEN_WITH_MOUSE_FOCUS    = -4,
		SCREEN_WITH_KEYBOARD_FOCUS = -3,
		SCREEN_PRIMARY             = -2,
		SCREEN_OF_MAIN_WINDOW      = -1, };

	class WindowManager
	{
	public:
		TST_NON_COPYABLE(WindowManager)

		static constexpr int MAIN_WINDOW_ID = 0;

		WindowManager();
		static WindowManager *create();
		static WindowManager *get();

		virtual ~WindowManager() = default;

		virtual WindowID createWindow(EWindowMode mode, const Rect2I &rect, EWindowFlags flags, EVsyncMode vsync_mode, WindowID parent_id) = 0;
		virtual void     destroyWindow(WindowID window) = 0;
		virtual void     showWindow(WindowID window) = 0;
		virtual void     hideWindow(WindowID window) = 0;

		using WindowEventCallback = std::function<void(EWindowEvent event)>;
		virtual void setWindowEventCallback(WindowID window_id, const WindowEventCallback &callback) = 0;

		using InputEventCallback = std::function<void(std::shared_ptr<class InputEvent> event)>;
		virtual void setInputEventCallback(WindowID window_id, const InputEventCallback &callback) = 0;

		virtual int      getScreenCount() const = 0;
		virtual int      getPrimaryScreen() const = 0;
		virtual int      getKeyboardFocusScreen() const = 0;
		virtual int      getScreenFromRect(const Rect2 &rect) const = 0;
		virtual Vector2I getScreenPosition(int p_screen = SCREEN_OF_MAIN_WINDOW) const = 0;
		virtual Size2I   getScreenSize(int p_screen = SCREEN_OF_MAIN_WINDOW) const = 0;
		virtual Rect2I   getScreenUsableRect(int p_screen = SCREEN_OF_MAIN_WINDOW) const = 0;
		virtual int      getScreenDPI(int p_screen = SCREEN_OF_MAIN_WINDOW) const = 0;
		virtual float    getScreenRefreshRate(int p_screen = SCREEN_OF_MAIN_WINDOW) const = 0;

		virtual void processEvents() = 0;
		virtual void swapBuffers() = 0;

	protected:
		int _getScreenIndex(int p_screen) const
		{
			switch (p_screen)
			{
				case SCREEN_WITH_MOUSE_FOCUS:
				{
					const Rect2I rect = Rect2I(getMousePosition(), Vector2I(1, 1));
					return getScreenFromRect(rect);
				}
				break;
				case SCREEN_WITH_KEYBOARD_FOCUS:
				{
					return getKeyboardFocusScreen();
				}
				break;
				case SCREEN_PRIMARY:
				{
					return getPrimaryScreen();
				}
				break;
				case SCREEN_OF_MAIN_WINDOW:
				{
					return getCurrentScreen(MAIN_WINDOW_ID);
				}
				break;
				default:
				{
					return p_screen;
				}
				break;
			}
		}
	};
}
