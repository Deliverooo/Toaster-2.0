#include "application.hpp"

#include "toast_lib/events/window_event.hpp"

#include "input.hpp"
#include "toast_render/globals.hpp"

#include <algorithm>
#include <GLFW/glfw3.h>

#include "toast_gpu/vk/vk_logical_device.hpp"
#include "toast_gpu/vk/vk_swapchain.hpp"

namespace toaster
{
	Application::Application(const ApplicationCreateInfo &p_create_info, [[maybe_unused]] int32 p_argc, [[maybe_unused]] char **p_argv) : m_createInfo(p_create_info)
	{
		Window::initWindowingAPI();

		#pragma region create vulkan objects
		gpu::VKInstanceSpecInfo vk_instance_spec_info{};
		vk_instance_spec_info.appName            = "Toaster-2.0 -> Vulkan QT";
		vk_instance_spec_info.requiredExtensions = Window::getRequiredInstanceExtensions();
		m_vkInstance                             = new gpu::VKInstance{vk_instance_spec_info};

		std::unordered_set<String> required_device_extensions{
			vk::KHRSwapchainExtensionName,
			vk::KHRDynamicRenderingExtensionName,
			vk::KHRTimelineSemaphoreExtensionName,
			vk::EXTCustomBorderColorExtensionName,
			vk::KHRMaintenance6ExtensionName,
			vk::KHRLoadStoreOpNoneExtensionName
		};
		gpu::VKPhysicalDeviceSpecInfo vk_physical_device_spec_info{};
		vk_physical_device_spec_info.requiredExtensions = required_device_extensions;

		m_vkPhysicalDevice = new gpu::VKPhysicalDevice{m_vkInstance, vk_physical_device_spec_info};

		gpu::VKLogicalDeviceSpecInfo vk_logical_device_spec_info{};
		vk_logical_device_spec_info.usePresent         = true;
		vk_logical_device_spec_info.requiredExtensions = required_device_extensions;
		auto features{gpu::VKLogicalDeviceSpecInfo::getDefaultFeatures()};
		vk_logical_device_spec_info.pNext = features.get<vk::PhysicalDeviceFeatures2>();

		m_vkLogicalDevice = new gpu::VKLogicalDevice{m_vkPhysicalDevice, vk_logical_device_spec_info};
		#pragma endregion

		#pragma region create window
		m_window = new Window{m_vkLogicalDevice, p_create_info.windowCreateInfo};
		m_window->setEventCallback([this](Event &e)
		{
			EventDispatcher dispatcher{e};
			dispatcher.dispatch<WindowCloseEvent>(TST_BIND_EVENT_FN(Application::onWindowCloseEvent));
			dispatcher.dispatch<WindowResizeEvent>(TST_BIND_EVENT_FN(Application::onWindowResizeEvent));

			std::ranges::for_each(m_layers.rbegin(), m_layers.rend(), [&](IAppLayer *layer)
			{
				if (e.isHandled())
					return;
				layer->onEvent(e);
			});
		});
		input::setCurrentWindowContext(m_window);
		#pragma endregion

		Globals::init(m_vkLogicalDevice);
	}

	Application::~Application() noexcept
	{
		m_vkLogicalDevice->getVulkanLogicalDevice().waitIdle();

		m_vkLogicalDevice->performGarbageCollection(); // Collect the trash from the layers
		for (IAppLayer *layer: m_layers)
			removeLayer(layer);
		m_layers.clear();

		Globals::shutdown();
		m_vkLogicalDevice->performGarbageCollection(); // Collect the trash from the globals

		delete m_window;
		Window::shutdownWindowingAPI();
		input::setCurrentWindowContext(nullptr);

		delete m_vkLogicalDevice;
		delete m_vkPhysicalDevice;
		delete m_vkInstance;
	}

	auto Application::run() -> void
	{
		while (m_isRunning)
		{
			const auto startTime{static_cast<float32>(glfwGetTime())};
			m_deltaTime     = startTime - m_lastFrameTime;
			m_lastFrameTime = startTime;

			m_window->processEvents();
			m_window->beginFrame();

			if (!m_minimized)
			{
				for (IAppLayer *layer: m_layers)
					layer->onUpdate(m_deltaTime);

				if (m_cbBeginUIRender)
					m_cbBeginUIRender();

				for (IAppLayer *layer: m_layers)
					layer->onUIRender();

				if (m_cbEndUIRender)
					m_cbEndUIRender();
			}
			m_window->endFrame();
		}
	}

	auto Application::close() noexcept -> void
	{
		m_isRunning = false;
	}

	auto Application::getWindow() const noexcept -> Window &
	{
		return *m_window;
	}

	auto Application::getLogicalDevice() const -> gpu::VKLogicalDevice *
	{
		return m_vkLogicalDevice;
	}

	auto Application::onWindowCloseEvent([[maybe_unused]] WindowCloseEvent &p_event) -> bool
	{
		m_isRunning = false;
		return true;
	}

	auto Application::onWindowResizeEvent(WindowResizeEvent &p_event) -> bool
	{
		const uint32 width{p_event.getWidth()};
		const uint32 height{p_event.getHeight()};

		if (width == 0 || height == 0)
		{
			m_minimized = true;
			return false;
		}

		m_minimized = false;
		return false;
	}

	auto Application::addLayer(IAppLayer *p_layer) -> void
	{
		m_layers.push_back(p_layer);
		p_layer->onInit();
	}

	auto Application::removeLayer(IAppLayer *p_layer) -> void
	{
		p_layer->onDestroy();
		m_layers.erase(std::ranges::find(m_layers, p_layer));
		delete p_layer;
	}

	auto Application::setBeginUIRenderCallback(const std::function<void()> &p_cb_begin_ui_render) -> void
	{
		m_cbBeginUIRender = p_cb_begin_ui_render;
	}

	auto Application::setEndUIRenderCallback(const std::function<void()> &p_cb_end_ui_render) -> void
	{
		m_cbEndUIRender = p_cb_end_ui_render;
	}
}
