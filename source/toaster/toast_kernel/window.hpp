#pragma once

#include <string>
#include <vulkan/vulkan.hpp>
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

		Window(uint32 p_width, uint32 p_height, std::string p_title);
		~Window();

		void beginFrame();
		void processEvents();
		void endFrame();

		void showWindow();
		void hideWindow();

		uint32             getWidth() const;
		uint32             getHeight() const;
		const std::string &getTitle() const;

		gpu::GPUContext *getGPUContext() const;
		gpu::Swapchain * getSwapchain() const;
		GLFWwindow *     getNativeWindow() const;

	private:
		gpu::GPUContext *m_gpuContext{nullptr};
		gpu::Swapchain * m_swapchain{nullptr};

		vk::SurfaceKHR m_windowSurface{nullptr};

		GLFWwindow *m_window{nullptr};

		struct GLFWCallbackData
		{
			uint32      width{0u};
			uint32      height{0u};
			std::string title;
		};

		GLFWCallbackData m_callbackData;
	};
}
