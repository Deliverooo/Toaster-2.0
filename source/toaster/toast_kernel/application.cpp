#include "application.hpp"

#include "input.hpp"
#include "toaster/toast_lib/logging.hpp"
#include "toaster/toast_render/globals.hpp"
#include "toaster/toast_render/render_command.hpp"

#include <algorithm>
#include <GLFW/glfw3.h>

namespace toaster
{
	Application::Application()
	{
		Window::initWindowingAPI();

		m_window = new Window(1280, 720, "Toaster: v0.314");

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

	bool Application::onWindowCloseEvent(WindowCloseEvent &e)
	{
		m_isRunning = false;
		return true;
	}

	bool Application::onWindowResizeEvent(WindowResizeEvent &e)
	{
		if (e.getWidth() == 0 || e.getHeight() == 0)
		{
			m_minimized = true;
			return false;
		}
		m_minimized = false;
		return false;
	}

	void Application::addLayer(IAppLayer *layer)
	{
		m_layers.push_back(layer);
		layer->onInit();
	}

	void Application::removeLayer(IAppLayer *layer)
	{
		layer->onDestroy();
		m_layers.erase(std::ranges::find(m_layers, layer));
		delete layer;
		layer = nullptr;
	}
}
