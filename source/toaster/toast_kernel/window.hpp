#pragma once

#include <string>

#include "events/event.hpp"

#if USE_VULKAN_BACKEND
#include <vulkan/vulkan.hpp>
#endif

#include "system_types.h"

struct GLFWwindow;

namespace toaster
{
	namespace gpu
	{
		class GPUContext;
		class Swapchain;
	}

	class Window
	{
	public:
		static void initWindowingAPI();
		static void shutdownWindowingAPI();

		Window(uint32 p_width, uint32 p_height, const std::string &p_title);
		~Window();

		void beginFrame();
		void processEvents();
		void endFrame();

		void showWindow();
		void hideWindow();

		void setEventCallback(const EventCallbackFn &p_callback);

		[[nodiscard]] uint32             getWidth() const;
		[[nodiscard]] uint32             getHeight() const;
		float32                          getAspect() const;
		[[nodiscard]] const std::string &getTitle() const;

		[[nodiscard]] gpu::GPUContext *getGPUContext() const;
		#if USE_VULKAN_BACKEND
		gpu::Swapchain *getSwapchain() const;
		#endif
		[[nodiscard]] GLFWwindow *getNativeWindow() const;

	private:
		gpu::GPUContext *m_gpuContext{nullptr};

		#if USE_VULKAN_BACKEND
		gpu::Swapchain *m_swapchain{nullptr}; vk::SurfaceKHR m_windowSurface{nullptr};
		#endif

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
