#include "window.hpp"

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

#include "toast_gpu/vk/vk_logical_device.hpp"
#include "toast_gpu/vk/vk_swapchain.hpp"

namespace toaster
{
	auto cstringArrayToVector(toaster::CString *p_arr, uint32 p_size) -> std::vector<toaster::CString>
	{
		std::vector<toaster::CString> vec{p_size};
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

	Window::Window(const WindowCreateInfo &p_create_info)
	{
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		// Hides the window during creation, as to not have a blank white screen
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

		glfwWindowHint(GLFW_SAMPLES, 4);

		m_callbackData.width  = p_create_info.width;
		m_callbackData.height = p_create_info.height;
		m_callbackData.title  = p_create_info.title;

		m_window = glfwCreateWindow(static_cast<int32>(p_create_info.width), static_cast<int32>(p_create_info.height), p_create_info.title.c_str(), nullptr, nullptr);

		uint32     glfw_extension_count{0u};
		const auto glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

		// Inserts the
		std::vector<CString> required_extensions{glfw_extension_count};
		for (uint32 i{0u}; i < glfw_extension_count; ++i)
			required_extensions[i] = glfw_extensions[i];

		required_extensions.emplace_back(vk::EXTDebugUtilsExtensionName);

		gpu::VKInstanceSpecInfo::ExtensionSet instance_extensions{required_extensions.begin(), required_extensions.end()};

		instance_extensions.insert(vk::KHRSurfaceExtensionName);
		gpu::VKInstanceSpecInfo vk_instance_spec_info{};
		vk_instance_spec_info.appName            = "Toaster-2.0 -> Vulkan QT";
		vk_instance_spec_info.requiredExtensions = instance_extensions;
		m_vkInstance                             = new gpu::VKInstance{vk_instance_spec_info};

		gpu::VKPhysicalDeviceSpecInfo vk_physical_device_spec_info{};
		vk_physical_device_spec_info.requiredExtensions = {
			vk::KHRSwapchainExtensionName,
			vk::KHRDynamicRenderingExtensionName,
			vk::KHRTimelineSemaphoreExtensionName,
			vk::EXTCustomBorderColorExtensionName,
			vk::KHRMaintenance6ExtensionName,
			vk::KHRLoadStoreOpNoneExtensionName
		};
		m_vkPhysicalDevice = new gpu::VKPhysicalDevice{m_vkInstance, vk_physical_device_spec_info};

		VkSurfaceKHR surface;
		if (auto err = glfwCreateWindowSurface(*m_vkInstance->getVulkanInstance(), m_window, nullptr, &surface); err != VK_SUCCESS)
		{
			LOG_ERROR("Failed to create window surface: {}", vk::to_string(static_cast<vk::Result>(err)));
			TST_ASSERT(false);
		}

		m_windowSurface = surface;
		gpu::VKLogicalDeviceSpecInfo vk_logical_device_spec_info{};
		vk_logical_device_spec_info.surface            = m_windowSurface;
		vk_logical_device_spec_info.requiredExtensions = {
			vk::KHRSwapchainExtensionName,
			vk::KHRDynamicRenderingExtensionName,
			vk::KHRTimelineSemaphoreExtensionName,
			vk::EXTCustomBorderColorExtensionName,
			vk::KHRMaintenance6ExtensionName,
			vk::KHRLoadStoreOpNoneExtensionName,
		};

		vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan12Features, vk::PhysicalDeviceVulkan13Features,
			vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT, vk::PhysicalDeviceCustomBorderColorFeaturesEXT> feature_chain{{}, {}, {}, {}, {}};
		feature_chain.get<vk::PhysicalDeviceFeatures2>().features.samplerAnisotropy = true;
		feature_chain.get<vk::PhysicalDeviceFeatures2>().features.sampleRateShading = true;
		feature_chain.get<vk::PhysicalDeviceFeatures2>().features.fillModeNonSolid = true;
		feature_chain.get<vk::PhysicalDeviceVulkan12Features>().timelineSemaphore = true;
		feature_chain.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering = true;
		feature_chain.get<vk::PhysicalDeviceVulkan13Features>().synchronization2 = true;
		feature_chain.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState = true;
		feature_chain.get<vk::PhysicalDeviceCustomBorderColorFeaturesEXT>().customBorderColors = true;
		vk_logical_device_spec_info.pNext = feature_chain.get<vk::PhysicalDeviceFeatures2>();
		m_vkLogicalDevice = new gpu::VKLogicalDevice{m_vkPhysicalDevice, vk_logical_device_spec_info};

		m_swapchain = new gpu::VKSwapchain(m_vkLogicalDevice, m_window);

		constexpr BOOL use_dark_mode{TRUE};
		(void) DwmSetWindowAttribute(glfwGetWin32Window(m_window), DWMWA_USE_IMMERSIVE_DARK_MODE, &use_dark_mode, sizeof(use_dark_mode));

		glfwSetWindowUserPointer(m_window, &m_callbackData);

		#define GET_CB_DATA() *(static_cast<GLFWCallbackData *>(glfwGetWindowUserPointer(window)))
		glfwSetWindowSizeCallback(m_window, [](GLFWwindow *window, const int32 width, const int32 height)
		{
			auto &data{GET_CB_DATA()};

			WindowResizeEvent event{static_cast<uint32>(width), static_cast<uint32>(height)};
			if (data.eventCallback)
				data.eventCallback(event);
			data.width  = width;
			data.height = height;
		});

		glfwSetWindowCloseCallback(m_window, [](GLFWwindow *window)
		{
			const auto &data{GET_CB_DATA()};

			WindowCloseEvent event{};
			if (data.eventCallback)
				data.eventCallback(event);
		});

		glfwSetKeyCallback(m_window, [](GLFWwindow *window, int32 key, [[maybe_unused]] int32 scancode, const int32 action, [[maybe_unused]] int32 mods)
		{
			const auto &data{GET_CB_DATA()};

			switch (action)
			{
				case GLFW_PRESS:
				{
					KeyPressEvent event{static_cast<input::EKeyCode>(key), 0};
					if (data.eventCallback)
						data.eventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleaseEvent event{static_cast<input::EKeyCode>(key)};
					if (data.eventCallback)
						data.eventCallback(event);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressEvent event{static_cast<input::EKeyCode>(key), 1};
					if (data.eventCallback)
						data.eventCallback(event);
					break;
				}
				default:
					break;
			}
		});

		glfwSetCharCallback(m_window, [](GLFWwindow *window, uint32_t codepoint)
		{
			const auto &data{GET_CB_DATA()};

			KeyTypeEvent event{static_cast<input::EKeyCode>(codepoint)};
			if (data.eventCallback)
				data.eventCallback(event);
		});

		glfwSetMouseButtonCallback(m_window, [](GLFWwindow *window, int32 button, int32 action, [[maybe_unused]] int32 mods)
		{
			const auto &data{GET_CB_DATA()};

			switch (action)
			{
				case GLFW_PRESS:
				{
					MouseButtonPressEvent event{static_cast<input::EMouseButton>(button)};
					if (data.eventCallback)
						data.eventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleaseEvent event{static_cast<input::EMouseButton>(button)};
					if (data.eventCallback)
						data.eventCallback(event);
					break;
				}
				default: break;
			}
		});

		glfwSetScrollCallback(m_window, [](GLFWwindow *window, float64 xOffset, float64 yOffset)
		{
			const auto &data{GET_CB_DATA()};

			MouseScrollEvent event{static_cast<float32>(xOffset), static_cast<float32>(yOffset)};
			if (data.eventCallback)
				data.eventCallback(event);
		});

		glfwSetCursorPosCallback(m_window, [](GLFWwindow *window, float64 x, float64 y)
		{
			const auto &data{GET_CB_DATA()};

			MouseMoveEvent event{static_cast<float32>(x), static_cast<float32>(y)};
			if (data.eventCallback)
				data.eventCallback(event);
		});

		glfwSetWindowMaximizeCallback(m_window, [](GLFWwindow *window, int32 maximized)
		{
			const auto &data{GET_CB_DATA()};

			WindowMaximizeEvent event{static_cast<bool>(maximized)};
			if (data.eventCallback)
				data.eventCallback(event);
		});

		glfwSetWindowIconifyCallback(m_window, [](GLFWwindow *window, int32 iconified)
		{
			const auto &data{GET_CB_DATA()};

			WindowMinimizeEvent event{static_cast<bool>(iconified)};
			if (data.eventCallback)
				data.eventCallback(event);
		});

		glfwSetDropCallback(m_window, [](GLFWwindow *window, int32 path_count, CString paths[])
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

		if (!p_create_info.iconPath.empty())
		{
			GLFWimage window_icon[1]{};

			int32 nr_channels{};
			window_icon[0].pixels = stbi_load(p_create_info.iconPath.string().c_str(), &window_icon[0].width, &window_icon[0].height, &nr_channels, 4);
			if (window_icon[0].pixels)
			{
				glfwSetWindowIcon(m_window, 1, window_icon);
				stbi_image_free(window_icon[0].pixels);
			}
			else
				LOG_ERROR("Failed to load image icon. Path: {}", p_create_info.iconPath.string());
		}

		if (p_create_info.startMaximized)
			maximize();

		showWindow();
	}

	Window::~Window()
	{
		m_vkLogicalDevice->getVulkanLogicalDevice().waitIdle();
		delete m_swapchain;

		vkDestroySurfaceKHR(*m_vkInstance->getVulkanInstance(), m_windowSurface, nullptr);

		delete m_vkLogicalDevice;
		delete m_vkPhysicalDevice;
		delete m_vkInstance;

		glfwDestroyWindow(m_window);
	}

	auto Window::beginFrame() -> void
	{
		m_swapchain->beginFrame();
	}

	auto Window::processEvents() -> void
	{
		glfwPollEvents();
	}

	auto Window::endFrame() -> void
	{
		m_swapchain->endFrame();
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

	auto Window::fullscreen() -> void
	{
		GLFWmonitor *      monitor{glfwGetPrimaryMonitor()};
		const GLFWvidmode *mode{glfwGetVideoMode(monitor)};
		glfwSetWindowMonitor(m_window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
	}

	auto Window::setEventCallback(const EventCallbackFn &p_callback) -> void
	{
		m_callbackData.eventCallback = p_callback;
	}

	auto Window::getWidth() const -> uint32
	{
		return m_callbackData.width;
	}

	auto Window::getAspect() const -> float32
	{
		return static_cast<float32>(m_callbackData.height) / static_cast<float32>(m_callbackData.width);
	}

	auto Window::getCenter() const -> std::pair<float32, float32>
	{
		return {static_cast<float32>(m_callbackData.width) / 2.0f, static_cast<float32>(m_callbackData.height) / 2.0f};
	}

	auto Window::getHeight() const -> uint32
	{
		return m_callbackData.height;
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

	auto Window::getLogicalDevice() const -> gpu::VKLogicalDevice *
	{
		return m_vkLogicalDevice;
	}

	auto Window::getNativeWindow() const -> GLFWwindow *
	{
		return m_window;
	}

	auto Window::getSwapchain() const -> gpu::VKSwapchain *
	{
		return m_swapchain;
	}
}
