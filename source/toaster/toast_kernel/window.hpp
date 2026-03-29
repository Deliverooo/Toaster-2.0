/*!
* @file window.hpp
 */
#pragma once

#include <string>

#include "toast_lib/string.hpp"
#include "toast_lib/system_types.h"
#include "toast_lib/events/event.hpp"
#include "toast_lib/io/filesystem.hpp"

struct GLFWwindow;

namespace toaster
{
	namespace gpu
	{
		class IGPUContext;
	}

	struct ScreenPos
	{
		float32 x, y;
	};

	struct WindowCreateInfo
	{
		uint32 width{1920u};
		uint32 height{1080u};
		String title{""};

		io::filesystem::Path iconPath{""};

		bool startMaximized{false};
	};

	/*!
	 * @class Window
	 *
	 * @brief Represents the window of the application
	 */
	class Window
	{
	public:
		/*!
		 * @brief Initializes the windowing API (GLFW)
		 *
		 * @details Called once before window creation in the Application class
		 */
		static void initWindowingAPI();
		/*!
		 * @brief Shuts down the windowing API (GLFW)
		 *
 		 * @details Called once after window destruction in the Application class
 		 */
		static void shutdownWindowingAPI();

		Window(const WindowCreateInfo &p_create_info);
		~Window();

		void beginFrame();
		void processEvents();
		void endFrame();

		void showWindow();
		void hideWindow();

		void maximize();
		void minimize();
		void restore();

		void fullscreen();

		void setEventCallback(const EventCallbackFn &p_callback);

		[[nodiscard]] uint32        getWidth() const;
		[[nodiscard]] uint32        getHeight() const;
		[[nodiscard]] float32       getAspect() const;
		[[nodiscard]] ScreenPos     getCenter() const;
		[[nodiscard]] const String &getTitle() const;

		void setTitle(const String &p_title);

		[[nodiscard]] gpu::IGPUContext *getGPUContext() const;

		[[nodiscard]] GLFWwindow *getNativeWindow() const;

	private:
		gpu::IGPUContext *m_gpuContext{nullptr};

		GLFWwindow *m_window{nullptr};

		struct GLFWCallbackData
		{
			uint32 width{0u};
			uint32 height{0u};
			String title;

			EventCallbackFn eventCallback;
		};

		GLFWCallbackData m_callbackData;
	};
}
