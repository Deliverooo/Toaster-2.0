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
#include "string/string.hpp"
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

#include "gpu/device_manager.hpp"

namespace tst
{
	namespace gpu
	{
		// Forward declare
		class VulkanSwapChain;
		class DeviceManager;
	}

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
		String title;
		uint32 width      = 0u;
		uint32 height     = 0u;
		bool   fullscreen = false;
	};

	class Window
	{
	public:
		explicit Window(const WindowSpecInfo &window_spec_info);
		~Window();

		EError init();

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

		[[nodiscard]] uint32 getWidth() const;
		[[nodiscard]] uint32 getHeight() const;
		[[nodiscard]] Vec2i  getSize() const;

		void setWindowSize(const Vec2i &size);
		void setWindowMinSize(const Vec2i &min_size);
		void setWindowMaxSize(const Vec2i &max_size);

		[[nodiscard]] Vec2i getPosition() const;
		void                setPosition(const Vec2i &pos);

		[[nodiscard]] const String &getTitle() const;
		void                        setTitle(const String &title);

		void minimize();
		void maximize();
		void centerWindow();

		[[nodiscard]] GLFWwindow *getNativeWindow() const;
		gpu::VulkanSwapChain *    getSwapchain();

		gpu::DeviceManager *getDeviceManager();

		#ifdef TST_PLATFORM_WINDOWS
		[[nodiscard]] HWND getHWND() const;
		#endif

	private:
		EError _createWindowSurface();
		void   _setGLFWCallbacks();

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

		gpu::DeviceManager *m_deviceManager;

		#ifdef TST_ENABLE_VULKAN
		gpu::VulkanSwapChain *m_vulkanSwapchain;
		vk::SurfaceKHR        m_windowSurface;
		#endif

		uint32 m_windowFlags;
	};
}
