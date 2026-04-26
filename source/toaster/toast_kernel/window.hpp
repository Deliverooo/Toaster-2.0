/*!
* @file window.hpp
 */
#pragma once

#include "../toaster_export.hpp"

#include <unordered_set>
#include <utility> // std::pair
#include <vulkan/vulkan_raii.hpp>

#include "toast_lib/string.hpp"
#include "toast_lib/system_types.h"
#include "toast_lib/events/event.hpp"
#include "toast_lib/io/filesystem.hpp"

struct GLFWwindow;

namespace toaster
{
	class InputContext;

	namespace gpu
	{
		class VKSwapchain;
		class VKInstance;
		class VKPhysicalDevice;
		class VKLogicalDevice;
	}

	struct TST_API WindowCreateInfo
	{
		uint32 width{1920u};
		uint32 height{1080u};
		String title{};

		io::filesystem::Path iconPath{};

		bool startMaximized{false};
	};

	/*!
	 * @class Window
	 *
	 * @brief Represents the window of the application
	 */
	class TST_API Window
	{
	public:
		/*!
		 * @brief Initializes the windowing API (GLFW)
		 *
		 * @details Called once before window creation in the Application class
		 */
		static auto initWindowingAPI() -> void;
		/*!
		 * @brief Shuts down the windowing API (GLFW)
		 *
 		 * @details Called once after window destruction in the Application class
 		 */
		static auto shutdownWindowingAPI() -> void;

		static auto getRequiredInstanceExtensions() -> std::unordered_set<String>;

		Window(gpu::VKLogicalDevice *p_device, const WindowCreateInfo &p_create_info);
		~Window();

		auto beginFrame() -> void;
		auto processEvents() -> void;
		auto endFrame() -> void;

		auto showWindow() -> void;
		auto hideWindow() -> void;

		auto maximize() -> void;
		auto minimize() -> void;
		auto restore() -> void;

		auto fullscreen() -> void;

		auto setEventCallback(const EventCallbackFn &p_callback) -> void;

		[[nodiscard]] auto getWidth() const -> uint32;
		[[nodiscard]] auto getHeight() const -> uint32;
		[[nodiscard]] auto getAspect() const -> float32;
		[[nodiscard]] auto getCenter() const -> std::pair<float32, float32>;
		[[nodiscard]] auto getTitle() const -> const String &;

		auto setTitle(const String &p_title) -> void;

		[[nodiscard]] auto getNativeWindow() const -> GLFWwindow *;
		[[nodiscard]] auto getSwapchain() const -> gpu::VKSwapchain *;
		[[nodiscard]] auto getInputContext() const -> InputContext *;

	private:
		gpu::VKLogicalDevice *m_device{nullptr};

		vk::SurfaceKHR m_windowSurface{nullptr};

		GLFWwindow *m_window{nullptr};

		InputContext *m_inputCtx{nullptr};

		gpu::VKSwapchain *m_swapchain{nullptr};

		struct GLFWCallbackData
		{
			uint32 width{0u};
			uint32 height{0u};
			String title{};

			EventCallbackFn eventCallback{nullptr};
		};

		GLFWCallbackData m_callbackData{};
	};
}
