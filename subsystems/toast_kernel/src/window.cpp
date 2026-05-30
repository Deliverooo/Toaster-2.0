#include "toast_kernel/window.hpp"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <dwmapi.h>
#include <GLFW/glfw3native.h>

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

#include <stb/stb_image.h>

#include "toast_lib/logging.hpp"
#include "toast_lib/toast_assert.h"

#include "toast_lib/events/key_event.hpp"
#include "toast_lib/events/mouse_event.hpp"
#include "toast_lib/events/window_event.hpp"

#include "toast_gpu/vk/vk_swapchain.hpp"

#include "toast_kernel/input.hpp"

#include <Windows.h>
#include <vulkan/vulkan_win32.h>

#include "toast_render/render_context.hpp"

namespace toaster
{
	auto cstringArrayToVector(CString *p_arr, uint32 p_size) -> std::vector<CString>
	{
		std::vector<CString> vec{p_size};
		for (uint32 i{0u}; i < p_size; ++i)
			vec.emplace_back(p_arr[i]);
		return vec;
	}

	static bool s_glfwInitialized{false};

	static auto _glfwErrorCallback(int32 error, CString description) -> void
	{
		LOG_ERROR("GLFW error: ({}): {}", error, description);
	}

	auto Window::initWindowingAPI() -> void
	{
		if (!s_glfwInitialized)
		{
			const bool init_result{static_cast<bool>(glfwInit())};
			(void) init_result;
			TST_ASSERT_MSG(init_result, "glfw initialization failed!");

			glfwSetErrorCallback(_glfwErrorCallback);

			s_glfwInitialized = true;
		}
	}

	auto Window::shutdownWindowingAPI() -> void
	{
		TST_ASSERT_MSG(s_glfwInitialized, "Attempted to shutdown windowing API before initializing it!");

		glfwTerminate();
	}

	auto Window::getRequiredInstanceExtensions() -> std::unordered_set<String>
	{
		uint32     glfw_extension_count{0u};
		const auto glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

		std::vector<String> required_extensions{glfw_extension_count};
		for (uint32 i{0u}; i < glfw_extension_count; ++i)
			required_extensions[i] = glfw_extensions[i];

		required_extensions.emplace_back(vk::KHRSurfaceExtensionName);
		required_extensions.emplace_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);

		return {required_extensions.begin(), required_extensions.end()};
	}

	Window::Window(render::RenderContext *p_render_ctx, const WindowSpecInfo &p_spec_info) : m_renderCtx(p_render_ctx)
	{
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		// Hides the window during creation, as to not have a blank white screen
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

		glfwWindowHint(GLFW_SAMPLES, 4);

		m_callbackData.size  = p_spec_info.size;
		m_callbackData.title = p_spec_info.title;

		m_window = glfwCreateWindow(static_cast<int32>(p_spec_info.size.x), static_cast<int32>(p_spec_info.size.y), p_spec_info.title.c_str(), nullptr, nullptr);

		#pragma region setup swapchain
		VkSurfaceKHR surface;
		if (auto err = glfwCreateWindowSurface(*m_renderCtx->getBackendInstance()->getVulkanInstance(), m_window, nullptr, &surface); err != VK_SUCCESS)
		{
			LOG_ERROR("Failed to create window surface: {}", vk::to_string(static_cast<vk::Result>(err)));
			TST_ASSERT(false);
		}
		m_windowSurface = surface;

		m_swapchain = new gpu::VKSwapchain(m_renderCtx->getLogicalDevice(), &m_windowSurface);
		m_swapchain->setGetWindowBackBufferSizeCallback([this]()
		{
			int32 width;
			int32 height;
			glfwGetFramebufferSize(m_window, &width, &height);
			return std::make_pair(static_cast<uint32>(width), static_cast<uint32>(height));
		});
		m_swapchain->setHandleMinimisationCallback([this]() -> void
		{
			int32 width;
			int32 height;
			glfwGetFramebufferSize(m_window, &width, &height);
			while (width == 0 || height == 0)
			{
				glfwGetFramebufferSize(m_window, &width, &height);
				glfwWaitEvents();
			}
		});

		m_swapchain->setUserDataPointer(m_renderCtx);
		m_swapchain->setBeginFrameCallback(+[](void *p_user_data, const uint32 frame_index) -> void
		{
			auto ctx{static_cast<render::RenderContext *>(p_user_data)};
			ctx->setCurrentFrameIndex(frame_index);
			ctx->performGarbageCollection();
		});
		#pragma endregion

		constexpr BOOL use_dark_mode{TRUE};
		(void) DwmSetWindowAttribute(glfwGetWin32Window(m_window), DWMWA_USE_IMMERSIVE_DARK_MODE, &use_dark_mode, sizeof(use_dark_mode));

		#pragma region setup glfw callbacks

		m_inputCtx                = new InputContext{this};
		m_callbackData.cbInputCtx = m_inputCtx;

		glfwSetWindowUserPointer(m_window, &m_callbackData);

		#define GET_CB_DATA() *(static_cast<GLFWCallbackData *>(glfwGetWindowUserPointer(window)))

		glfwSetKeyCallback(m_window, +[](GLFWwindow *window, int32 key, [[maybe_unused]] int32 scancode, const int32 action, [[maybe_unused]] int32 mods)
		{
			auto &data{GET_CB_DATA()};

			switch (action)
			{
				case GLFW_PRESS:
				{
					data.cbInputCtx->_setKeyState(static_cast<input::EKeyCode>(key), input::EKeyState::ePressed);
					KeyPressEvent event{static_cast<input::EKeyCode>(key), 0};
					if (data.eventCallback)
						data.eventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					data.cbInputCtx->_setKeyState(static_cast<input::EKeyCode>(key), input::EKeyState::eReleased);
					KeyReleaseEvent event{static_cast<input::EKeyCode>(key)};
					if (data.eventCallback)
						data.eventCallback(event);
					break;
				}
				case GLFW_REPEAT:
				{
					data.cbInputCtx->_setKeyState(static_cast<input::EKeyCode>(key), input::EKeyState::eHeld);
					KeyPressEvent event{static_cast<input::EKeyCode>(key), 1};
					if (data.eventCallback)
						data.eventCallback(event);
					break;
				}
				default:
					break;
			}
		});

		glfwSetWindowSizeCallback(m_window, +[](GLFWwindow *window, const int32 width, const int32 height)
		{
			auto &data{GET_CB_DATA()};

			WindowResizeEvent event{static_cast<uint32>(width), static_cast<uint32>(height)};
			if (data.eventCallback)
				data.eventCallback(event);
			data.size = {static_cast<uint32>(width), static_cast<uint32>(height)};
		});

		glfwSetWindowCloseCallback(m_window, +[](GLFWwindow *window)
		{
			const auto &data{GET_CB_DATA()};

			WindowCloseEvent event{};
			if (data.eventCallback)
				data.eventCallback(event);
		});

		glfwSetCharCallback(m_window, +[](GLFWwindow *window, uint32_t codepoint)
		{
			const auto &data{GET_CB_DATA()};

			KeyTypeEvent event{static_cast<input::EKeyCode>(codepoint)};
			if (data.eventCallback)
				data.eventCallback(event);
		});

		glfwSetMouseButtonCallback(m_window, +[](GLFWwindow *window, int32 button, int32 action, [[maybe_unused]] int32 mods)
		{
			const auto &data{GET_CB_DATA()};

			switch (action)
			{
				case GLFW_PRESS:
				{
					data.cbInputCtx->_setMouseButtonState(static_cast<input::EMouseButton>(button), input::EKeyState::ePressed);
					MouseButtonPressEvent event{static_cast<input::EMouseButton>(button)};
					if (data.eventCallback)
						data.eventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					data.cbInputCtx->_setMouseButtonState(static_cast<input::EMouseButton>(button), input::EKeyState::eReleased);
					MouseButtonReleaseEvent event{static_cast<input::EMouseButton>(button)};
					if (data.eventCallback)
						data.eventCallback(event);
					break;
				}
				default: break;
			}
		});

		glfwSetScrollCallback(m_window, +[](GLFWwindow *window, float64 xOffset, float64 yOffset)
		{
			const auto &data{GET_CB_DATA()};

			data.cbInputCtx->_setMouseScroll(static_cast<float32>(xOffset), static_cast<float32>(yOffset));
			MouseScrollEvent event{static_cast<float32>(xOffset), static_cast<float32>(yOffset)};
			if (data.eventCallback)
				data.eventCallback(event);
		});

		glfwSetCursorPosCallback(m_window, +[](GLFWwindow *window, float64 x, float64 y)
		{
			const auto &data{GET_CB_DATA()};

			MouseMoveEvent event{static_cast<float32>(x), static_cast<float32>(y)};
			if (data.eventCallback)
				data.eventCallback(event);
		});

		glfwSetWindowMaximizeCallback(m_window, +[](GLFWwindow *window, int32 maximized)
		{
			const auto &data{GET_CB_DATA()};

			WindowMaximizeEvent event{static_cast<bool>(maximized)};
			if (data.eventCallback)
				data.eventCallback(event);
		});

		glfwSetWindowIconifyCallback(m_window, +[](GLFWwindow *window, int32 iconified)
		{
			const auto &data{GET_CB_DATA()};

			WindowMinimizeEvent event{static_cast<bool>(iconified)};
			if (data.eventCallback)
				data.eventCallback(event);
		});

		glfwSetDropCallback(m_window, +[](GLFWwindow *window, int32 path_count, CString paths[])
		{
			const auto &data{GET_CB_DATA()};

			std::vector<String> filepaths{static_cast<std::vector<String>::size_type>(path_count)};
			for (uint32 i{0u}; i < path_count; ++i)
			{
				LOG_INFO("File received: {}", paths[i]);
				filepaths[i] = paths[i];
			}
			WindowFileDropEvent event{filepaths};

			if (data.eventCallback)
				data.eventCallback(event);
		});
		#undef GET_CB_DATA
		#pragma endregion

		if (!p_spec_info.iconPath.empty())
		{
			GLFWimage window_icon[1]{};

			int32 nr_channels{};
			window_icon[0].pixels = stbi_load(p_spec_info.iconPath.string().c_str(), &window_icon[0].width, &window_icon[0].height, &nr_channels, 4);
			if (window_icon[0].pixels)
			{
				glfwSetWindowIcon(m_window, 1, window_icon);
				stbi_image_free(window_icon[0].pixels);
			}
			else
				LOG_ERROR("Failed to load image icon. Path: {}", p_spec_info.iconPath.string());
		}

		if (p_spec_info.startMaximized)
			maximize();

		showWindow();
	}

	Window::~Window()
	{
		m_renderCtx->gpuWaitIdle();
		delete m_swapchain;

		vkDestroySurfaceKHR(*m_renderCtx->getBackendInstance()->getVulkanInstance(), m_windowSurface, nullptr);

		delete m_inputCtx;
		m_callbackData.cbInputCtx = nullptr;

		glfwDestroyWindow(m_window);
	}

	auto Window::beginFrame() -> void
	{
		m_swapchain->beginFrame();
	}

	auto Window::processEvents() -> void
	{
		m_inputCtx->_update();
		glfwPollEvents();
	}

	auto Window::endFrame() -> void
	{
		m_swapchain->endFrame();
		m_inputCtx->_onEndFrame();
	}

	auto Window::showWindow() -> void
	{
		glfwShowWindow(m_window);
	}

	auto Window::hideWindow() -> void
	{
		glfwHideWindow(m_window);
	}

	auto Window::maximize() -> void
	{
		glfwMaximizeWindow(m_window);
	}

	auto Window::minimize() -> void
	{
		glfwIconifyWindow(m_window);
	}

	auto Window::restore() -> void
	{
		glfwRestoreWindow(m_window);
	}

	auto Window::isFullscreen() const -> bool
	{
		return glfwGetWindowMonitor(m_window);
	}

	auto Window::setFullscreen() -> void
	{
		GLFWmonitor *      monitor{glfwGetPrimaryMonitor()};
		const GLFWvidmode *mode{glfwGetVideoMode(monitor)};
		glfwSetWindowMonitor(m_window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
	}

	auto Window::setWindowed() -> void
	{
		if (glfwGetWindowMonitor(m_window))
		{
			glfwSetWindowMonitor(m_window, nullptr, 0, 0, static_cast<int32>(m_callbackData.size.x), static_cast<int32>(m_callbackData.size.y), 0);
		}
	}

	auto Window::setEventCallback(const EventCallbackFn &p_callback) -> void
	{
		m_callbackData.eventCallback = p_callback;
	}

	auto Window::getSize() const -> tsm::uint2
	{
		return m_callbackData.size;
	}

	auto Window::getAspect() const -> float32
	{
		return static_cast<float32>(m_callbackData.size.y) / static_cast<float32>(m_callbackData.size.x);
	}

	auto Window::getCenter() const -> std::pair<float32, float32>
	{
		return {static_cast<float32>(m_callbackData.size.x) / 2.0f, static_cast<float32>(m_callbackData.size.y) / 2.0f};
	}

	auto Window::getTitle() const -> const std::string &
	{
		return m_callbackData.title;
	}

	auto Window::setTitle(const std::string &p_title) -> void
	{
		m_callbackData.title = p_title;
		glfwSetWindowTitle(m_window, p_title.c_str());
	}

	auto Window::getNativeWindow() const -> GLFWwindow *
	{
		return m_window;
	}

	auto Window::getSwapchain() const -> gpu::VKSwapchain *
	{
		return m_swapchain;
	}

	auto Window::getInputContext() const -> InputContext *
	{
		return m_inputCtx;
	}

	auto Window::getSwapchainRenderingInfo(const tsm::float4 &p_clear_colour, bool p_use_depth, tsm::float2 p_clear_depth) const -> gpu::RenderingInfo
	{
		gpu::RenderingInfo rendering_info{};
		rendering_info.renderArea = vk::Rect2D{{0, 0}, {m_swapchain->getExtent().width, m_swapchain->getExtent().height}};

		auto &colour_attachment_info{rendering_info.colourAttachments.emplace_back()};
		colour_attachment_info.imageView   = m_swapchain->getCurrentImageView();
		colour_attachment_info.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
		colour_attachment_info.clearValue  = vk::ClearColorValue{p_clear_colour.x, p_clear_colour.y, p_clear_colour.z, p_clear_colour.w};

		if (p_use_depth)
		{
			gpu::RenderingAttachmentInfo depth_attachment_info{};
			depth_attachment_info.imageView   = m_swapchain->getDepthImageView();
			depth_attachment_info.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
			depth_attachment_info.clearValue  = vk::ClearDepthStencilValue{p_clear_depth.x, static_cast<uint32>(p_clear_depth.y)};
			rendering_info.depthAttachment    = depth_attachment_info;
		}
		return rendering_info;
	}
}
