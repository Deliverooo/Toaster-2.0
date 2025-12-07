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

#include "core_typedefs.hpp"
#include "string.hpp"
#include "events/event.hpp"
#include "math/vector.hpp"
#include "error.hpp"

#ifdef TST_ENABLE_VULKAN
#include <vulkan/vulkan.hpp>
#endif

#include <GLFW/glfw3.h>

#ifdef TST_PLATFORM_WINDOWS
#include <Windows.h>
#endif

#include <filesystem>

#include "gpu/device_manager.hpp"

namespace tst
{
	class VulkanSwapChain;

	class DeviceManager;

	enum class EVsyncMode : uint32
	{
		// Disables V-Sync
		eDisabled,
		// Enables V-Sync
		eEnabled,
		// If supported, enables adaptive V-Sync
		eAdaptive
	};

	enum class EWindowFlags : uint32
	{
		eResizeDisabled,
		// Disables resizing
		eBorderless,
		// Removes the window borders
		eAlwaysOnTop,
		// Shows the window above all other apps
		eTransparent,
		// Makes the window surface transparent
		eNoFocus,
		// Makes the window permanently unfocused
		eSharpCorners,
		// Makes the corners sharp
		eMinimizeDisabled,
		// Disables the minimize button
		eMaximizeDisabled
		// Disables the maximize button
	};

	struct WindowSpecInfo
	{
		String                title;
		uint32                width      = 0u;
		uint32                height     = 0u;
		bool                  fullscreen = false;
		bool                  vSync      = false;
		std::filesystem::path iconPath   = "";
	};

	class Window
	{
	public:
		explicit Window(const WindowSpecInfo &window_spec_info);
		~Window();

		void init();
		void terminate();

		void processEvents();

		void beginFrame();
		void present();

		void setEventCallback(const FnEventCallback &callback)
		{
			m_callbackData.cbFunction = callback;
		}

		[[nodiscard]] bool getWindowFlag(EWindowFlags window_flag) const;
		void               setWindowFlag(EWindowFlags window_flag, bool enable);

		[[nodiscard]] bool isVsyncEnabled() const;
		void               setVsyncMode(EVsyncMode vsync_mode);

		[[nodiscard]] uint32      getWidth() const;
		[[nodiscard]] uint32      getHeight() const;
		[[nodiscard]] tsm::vec2ui getSize() const;

		void setWindowSize(const tsm::vec2ui &size);
		void setWindowMinSize(const tsm::vec2ui &min_size);
		void setWindowMaxSize(const tsm::vec2ui &max_size);

		void setResizable(bool resizable);

		[[nodiscard]] tsm::vec2ui getPosition() const;
		void                     setPosition(const tsm::vec2ui &pos);

		[[nodiscard]] const String &getTitle() const;
		void                        setTitle(const String &title);

		void minimize();
		void maximize();
		void centerWindow();

		[[nodiscard]] GLFWwindow *getNativeWindow() const;
		VulkanSwapChain *         getSwapchain();

		DeviceManager *getDeviceManager();

		#ifdef TST_PLATFORM_WINDOWS
		[[nodiscard]] HWND getHWND() const;
		#endif

		void onWindowSizeCallback(int width, int height);
		void onWindowCloseCallback();
		void onKeyCallback(int key, int scancode, int action, int mods);
		void onCharCallback(uint32_t codepoint);
		void onMouseButtonCallback(int button, int action, int mods);
		void onMouseScrollCallback(double xOffset, double yOffset);
		void onMousePosCallback(double x, double y);
		void onTitlebarHitTestCallback(int x, int y, int *hit);
		void onWindowIconifyCallback(int iconified);

	private:
		void _createWindowSurface();
		void _setGLFWCallbacks();

		struct CallbackData
		{
			String          cbTitle;
			FnEventCallback cbFunction;

			uint32 cbWidth;
			uint32 cbHeight;
		};

		CallbackData   m_callbackData;
		WindowSpecInfo m_specInfo;

		GLFWwindow *m_windowHandle;
		GLFWcursor *m_ImGuiMouseCursors[9] = {0};

		DeviceManager *m_deviceManager;

		#ifdef TST_ENABLE_VULKAN
		VulkanSwapChain *m_vulkanSwapchain;
		vk::SurfaceKHR   m_windowSurface;
		#endif

		uint32 m_windowFlags;
	};
}
