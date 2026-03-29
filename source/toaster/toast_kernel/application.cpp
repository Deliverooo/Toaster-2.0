#include "application.hpp"

#include "input.hpp"
#include "toast_lib/logging.hpp"
#include "toast_render/globals.hpp"
#include "toast_render/render_command.hpp"

#include <algorithm>
#include <GLFW/glfw3.h>

namespace toaster
{
	Application::Application(const ApplicationCreateInfo &p_create_info) : m_createInfo(p_create_info)
	{
		Window::initWindowingAPI();

		m_window = new Window(p_create_info.windowCreateInfo);

		m_window->setEventCallback([this](Event &e)
		{
			EventDispatcher dispatcher(e);
			dispatcher.dispatch<WindowCloseEvent>(TST_BIND_EVENT_FN(Application::onWindowCloseEvent));
			dispatcher.dispatch<WindowResizeEvent>(TST_BIND_EVENT_FN(Application::onWindowResizeEvent));

			std::ranges::for_each(m_layers.rbegin(), m_layers.rend(), [&](IAppLayer *layer)
			{
				if (e.isHandled())
					return;
				layer->onEvent(e);
			});
		});

		Globals::init();
		RenderCommand::init();

		input::setCurrentWindowContext(m_window->getNativeWindow());
	}

	Application::~Application() noexcept
	{
		for (IAppLayer *layer: m_layers)
			removeLayer(layer);
		m_layers.clear();

		Globals::shutdown();

		delete m_window;
		Window::shutdownWindowingAPI();
	}

	void Application::run()
	{
		while (m_isRunning)
		{
			const auto startTime = static_cast<float32>(glfwGetTime());
			m_deltaTime          = startTime - m_lastFrameTime;
			m_lastFrameTime      = startTime;

			m_window->processEvents();
			m_window->beginFrame();

			if (!m_minimized)
			{
				for (IAppLayer *layer: m_layers)
				{
					layer->onUpdate(m_deltaTime);
				}

				if (m_cbBeginUIRender)
					m_cbBeginUIRender();

				for (IAppLayer *layer: m_layers)
				{
					layer->onUIRender();
				}

				if (m_cbEndUIRender)
					m_cbEndUIRender();
			}
			m_window->endFrame();
		}
	}

	void Application::close() noexcept
	{
		m_isRunning = false;
	}

	Window &Application::getWindow() const noexcept
	{
		return *m_window;
	}

	bool Application::onWindowCloseEvent([[maybe_unused]] WindowCloseEvent &p_event)
	{
		m_isRunning = false;
		return true;
	}

	bool Application::onWindowResizeEvent(WindowResizeEvent &p_event)
	{
		if (p_event.getWidth() == 0 || p_event.getHeight() == 0)
		{
			m_minimized = true;
			return false;
		}
		m_minimized = false;
		return false;
	}

	void Application::addLayer(IAppLayer *p_layer)
	{
		m_layers.push_back(p_layer);
		p_layer->onInit();
	}

	void Application::removeLayer(IAppLayer *p_layer)
	{
		p_layer->onDestroy();
		m_layers.erase(std::ranges::find(m_layers, p_layer));
		delete p_layer;
	}

	void Application::setBeginUIRenderCallback(const std::function<void()> &p_cb_begin_ui_render)
	{
		m_cbBeginUIRender = p_cb_begin_ui_render;
	}

	void Application::setEndUIRenderCallback(const std::function<void()> &p_cb_end_ui_render)
	{
		m_cbEndUIRender = p_cb_end_ui_render;
	}
}
