#include "application.hpp"

#include "events/window_event.hpp"
#include "renderer/renderer.hpp"

namespace tst
{
	Application::Application(std::vector<String> args) : m_renderThread(EThreadingPolicy::eMultiThreaded)
	{
		s_instance = this;

		m_renderThread.run();

		WindowSpecInfo window_spec{};
		window_spec.width      = 1440;
		window_spec.height     = 900;
		window_spec.title      = "Toast";
		window_spec.fullscreen = false;

		m_window = std::make_unique<Window>(window_spec);
		m_window->init();
		m_window->setEventCallback([&](Event &e)
		{
			_onEvent(e);
		});

		Renderer::init();
		m_renderThread.pump();

		m_window->centerWindow();
	}

	Application::~Application()
	{
		m_window->setEventCallback([](Event &e)
		{
		});

		m_renderThread.terminate();

		for (ILayer *layer: m_layerStack)
		{
			layer->onDestroy();
			delete layer;
		}

		Renderer::terminate();
	}

	void Application::_processEvents()
	{
		m_window->processEvents();
	}

	void Application::_onEvent(Event &event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.dispatch<WindowResizedEvent>([this](WindowResizedEvent &e) { return _onWindowResize(e); });
		dispatcher.dispatch<WindowMinimizedEvent>([this](WindowMinimizedEvent &e) { return _onWindowMinimize(e); });
		dispatcher.dispatch<WindowClosedEvent>([this](WindowClosedEvent &e) { return _onWindowClose(e); });

		for (auto it = m_layerStack.end(); it != m_layerStack.begin();)
		{
			(*--it)->onEvent(event);
			if (event.isHandled())
				break;
		}

		if (event.isHandled())
			return;
	}

	bool Application::_onWindowResize(WindowResizedEvent &event)
	{
		const uint32 width  = event.getWidth();
		const uint32 height = event.getHeight();

		if (width == 0 || height == 0)
		{
			return false;
		}

		return false;
	}

	bool Application::_onWindowMinimize(WindowMinimizedEvent &event)
	{
		m_minimized = event.isMinimized();
		return false;
	}

	bool Application::_onWindowClose(WindowClosedEvent &event)
	{
		m_isRunning = false;
		return false;
	}

	void Application::run()
	{
		while (m_isRunning)
		{
			m_renderThread.blockUntilRenderComplete();

			_processEvents();

			m_renderThread.nextFrame();

			m_renderThread.kick();

			if (!m_minimized)
			{
				Renderer::submit([&]()
				{
					m_window->beginFrame();
				});

				Renderer::beginFrame();
				{
					for (ILayer *layer: m_layerStack)
						layer->onUpdate(m_deltaTime);
				}

				Renderer::endFrame();
				Renderer::submit([&]()
				{
					m_window->present();
					getGraphicsDevice()->runGarbageCollection();
				});
			}
		}
	}

	void Application::pushLayer(ILayer *layer)
	{
		m_layerStack.pushLayer(layer);
		layer->onInit();
	}

	void Application::popLayer(ILayer *layer)
	{
		m_layerStack.popLayer(layer);
		layer->onDestroy();
	}
}
