#pragma once

#include "core/core.hpp"

namespace tst
{
	using WindowID = int;

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
		eBorderlessFullscreen, };

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

	class WindowManager
	{
	public:
		static constexpr int INVALID_WINDOW_ID = -1;
		static constexpr int MAIN_WINDOW_ID    = 0;

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

		virtual void processEvents() = 0;
		virtual void swapBuffers() = 0;
	};
}
