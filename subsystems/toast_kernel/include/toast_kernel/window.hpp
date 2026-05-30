/*!
* @file window.hpp
 */
#pragma once

#include "toast_kernel.hpp"

#include <unordered_set>
#include <utility> // std::pair
#include <vulkan/vulkan_raii.hpp>

#include "toast_gpu/vk/vk_render_attachment.hpp"
#include "toast_lib/ptr.hpp"
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
	}

	namespace render
	{
		class RenderContext;
	}

	struct TST_KERNEL_API WindowSpecInfo
	{
		tsm::uint2 size{1920u, 1080u};
		String     title{};

		io::filesystem::Path iconPath{};

		bool startMaximized{false};
	};

	/*!
	 * @class Window
	 *
	 * @brief Represents the window of the application
	 */
	class TST_KERNEL_API Window
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

		Window(render::RenderContext *p_render_ctx, const WindowSpecInfo &p_spec_info);
		~Window();

		auto beginFrame() -> void;
		auto processEvents() -> void;
		auto endFrame() -> void;

		auto showWindow() -> void;
		auto hideWindow() -> void;

		auto maximize() -> void;
		auto minimize() -> void;
		auto restore() -> void;

		auto isFullscreen() const -> bool;
		auto setFullscreen() -> void;
		auto setWindowed() -> void;

		auto setEventCallback(const EventCallbackFn &p_callback) -> void;

		[[nodiscard]] auto getSize() const -> tsm::uint2;
		[[nodiscard]] auto getAspect() const -> float32;
		[[nodiscard]] auto getCenter() const -> std::pair<float32, float32>;
		[[nodiscard]] auto getTitle() const -> const String &;

		auto setTitle(const String &p_title) -> void;

		[[nodiscard]] auto getNativeWindow() const -> GLFWwindow *;
		[[nodiscard]] auto getSwapchain() const -> gpu::VKSwapchain *;
		[[nodiscard]] auto getInputContext() const -> InputContext *;

		[[nodiscard]] auto getSwapchainRenderingInfo(const tsm::float4 &p_clear_colour, bool p_use_depth = true,
													 tsm::float2        p_clear_depth                    = {1.0f, 0.0f}) const -> gpu::RenderingInfo;

	private:
		NonOwningPtr<render::RenderContext> m_renderCtx{nullptr};

		vk::SurfaceKHR m_windowSurface{nullptr};

		GLFWwindow *m_window{nullptr};

		InputContext *m_inputCtx{nullptr};

		gpu::VKSwapchain *m_swapchain{nullptr};

		struct GLFWCallbackData
		{
			tsm::uint2 size{0u};
			String     title{};

			EventCallbackFn eventCallback{nullptr};
			InputContext *  cbInputCtx{nullptr};
		};

		GLFWCallbackData m_callbackData{};
	};
}
