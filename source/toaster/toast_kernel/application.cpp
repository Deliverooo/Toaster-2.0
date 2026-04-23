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

		m_window = new Window{p_create_info.windowCreateInfo};

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

		Globals::init(m_window->getLogicalDevice());

		input::setCurrentWindowContext(m_window->getNativeWindow());
	}

	Application::~Application() noexcept
	{
		const auto device{m_window->getLogicalDevice()};
		device->getVulkanLogicalDevice().waitIdle();

		for (IAppLayer *layer: m_layers)
			removeLayer(layer);
		m_layers.clear();

		Globals::shutdown();

		device->performGarbageCollection();

		delete m_window;
		Window::shutdownWindowingAPI();
	}

	auto Application::run() -> void
	{
		while (m_isRunning)
		{
			const auto startTime{static_cast<float32>(glfwGetTime())};
			m_deltaTime     = startTime - m_lastFrameTime;
			m_lastFrameTime = startTime;

			m_window->processEvents();

			// TODO: Add some kind of callback thing so we can handle the creation and recreation of renderer resources.
			// Example from Hazel, I'm not sure about doing things this way because I don't like any form of static interfaces or coupling
			// Ref<VulkanTexture2D> instance = this; Renderer::Submit([instance]() mutable { instance->Invalidate(); });

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

		m_window->getSwapchain()->setFramebufferResized(true);

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
