/*
*  	Copyright 2025 Orbo Stetson
 *  	Licensed under the Apache License, Version 2.0 (the "License");
 *  	you may not use this file except in compliance with the License.
 *  	You may obtain a copy of the License at
 *
 *			http://www.apache.org/licenses/LICENSE-2.0
 *
 *    	Unless required by applicable law or agreed to in writing, software
 *    	distributed under the License is distributed on an "AS IS" BASIS,
 *    	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    	See the License for the specific language governing permissions and
 *    	limitations under the License.
 */
#pragma once

#include "platform/window_manager.hpp"

#include <Windows.h>

namespace tst
{
	namespace gpu
	{
		class RenderingContext;
		class RenderingDevice;
	}

	struct WindowsKeyEvent
	{
		WindowID windowId;
		bool     alt;
		bool     shift;
		bool     control;
		bool     meta;
		bool     altGr;
		UINT     uMsg;
		WPARAM   wParam;
		LPARAM   lParam;
	};

	constexpr int KEY_EVENT_BUFFER_SIZE = 512;

	class WindowsWindowManager : public WindowManager
	{
	public:
		WindowsWindowManager();
		~WindowsWindowManager() override;

		// std::vector<WindowID> getWindowList() const override;

		WindowID createWindow(EWindowMode mode, const Rect2I &rect, EWindowFlags flags, EVsyncMode vsync_mode, WindowID parent_id) override;
		void     destroyWindow(WindowID window) override;
		void     showWindow(WindowID window) override;
		void     hideWindow(WindowID window) override;

		void setWindowEventCallback(WindowID window_id, const WindowEventCallback &callback) override;
		void setInputEventCallback(WindowID window_id, const InputEventCallback &callback) override;

		void processEvents() override;
		void swapBuffers() override;

		LRESULT WndProcFileDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT MouseProc(int code, WPARAM wParam, LPARAM lParam);

		// void     setWindowTitle(WindowID windowID, const std::string &title);
		// void     setWindowPosition(WindowID windowID, const Vector2I &position);
		// void     setWindowSize(WindowID windowID, const Vector2I &size);
		// Vector2I getWindowPosition(WindowID windowID);
		// Vector2I getWindowSize(WindowID windowID);

	private:
		void getWindowStyle(bool main_window, bool embed_child, bool initialized, bool fullscreen, bool borderless, bool resizable, bool no_min_btn, bool no_max_btn,
							bool minimized, bool   maximized, bool   no_activate_focus, DWORD &out_style, DWORD &out_style_ex);

		WindowID m_mouseOverWindowID = INVALID_WINDOW_ID;

		struct WindowData
		{
			HWND     hWnd;
			WindowID id;
			bool     maximized     = false;
			bool     minimized     = false;
			bool     fullscreen    = false;
			bool     borderless    = false;
			bool     resizable     = true;
			bool     windowFocused = false;

			Vector2I min_size;
			Vector2I max_size;
			int      width  = 0;
			int      height = 0;

			WindowEventCallback eventCallback;
			InputEventCallback  inputEventCallback;

			HWND parentWindow = nullptr;
		};

		std::unordered_map<WindowID, WindowData> m_windows;
		WindowID                                 m_windowIdCounter = 0;

		HINSTANCE   m_hInstance;
		WNDCLASSEXW m_windowClass;

		WindowsKeyEvent m_keyEventBuffer[KEY_EVENT_BUFFER_SIZE] = {};

		String m_graphicsAPI;

		gpu::RenderingContext *m_renderingContext = nullptr;
		gpu::RenderingDevice * m_renderingDevice  = nullptr;
	};
}
