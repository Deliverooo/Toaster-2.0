/*!
* @file window.hpp
 */
#pragma once

#include <string>

#include "events/event.hpp"

#include "system_types.h"

struct GLFWwindow;

namespace toaster
{
	namespace gpu
	{
		class GPUContext;
	}

	struct ScreenPos
	{
		float32 x, y;
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

		Window(uint32 p_width, uint32 p_height, const std::string &p_title);
		~Window();

		void beginFrame();
		void processEvents();
		void endFrame();

		void showWindow();
		void hideWindow();

		void maximize();
		void minimize();
		void restore();

		void setEventCallback(const EventCallbackFn &p_callback);

		[[nodiscard]] uint32             getWidth() const;
		[[nodiscard]] uint32             getHeight() const;
		[[nodiscard]] float32            getAspect() const;
		[[nodiscard]] ScreenPos          getCenter() const;
		[[nodiscard]] const std::string &getTitle() const;

		[[nodiscard]] gpu::GPUContext *getGPUContext() const;

		[[nodiscard]] GLFWwindow *getNativeWindow() const;

	private:
		gpu::GPUContext *m_gpuContext{nullptr};

		GLFWwindow *m_window{nullptr};

		struct GLFWCallbackData
		{
			uint32      width{0u};
			uint32      height{0u};
			std::string title;

			EventCallbackFn eventCallback;
		};

		GLFWCallbackData m_callbackData;
	};
}
