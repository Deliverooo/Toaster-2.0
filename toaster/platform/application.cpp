#include "application.hpp"

#include "events/window_event.hpp"
#include "renderer/renderer.hpp"

namespace tst
{
	Application::Application(const ApplicationSpecInfo &spec_info) : m_specInfo(spec_info), m_renderThread(spec_info.CoreThreadingPolicy)
	{
		s_instance = this;

		m_renderThread.run();

		if (!spec_info.WorkingDirectory.empty())
			std::filesystem::current_path(spec_info.WorkingDirectory);

		Renderer::setConfigInfo(spec_info.RenderConfig);

		WindowSpecInfo window_spec{};
		window_spec.title      = spec_info.Name;
		window_spec.width      = spec_info.WindowWidth;
		window_spec.height     = spec_info.WindowHeight;
		window_spec.fullscreen = spec_info.Fullscreen;
		window_spec.vSync      = spec_info.VSync;
		window_spec.iconPath   = spec_info.IconPath;

		m_window = std::make_unique<Window>(window_spec);
		m_window->init();
		m_window->setEventCallback([this](Event &e)
		{
			_onEvent(e);
		});

		Renderer::init();
		m_renderThread.pump();

		if (spec_info.StartMaximized)
		{
			m_window->maximize();
		}
		else
		{
			m_window->centerWindow();
		}

		m_window->setResizable(spec_info.Resizable);

		if (m_enableImGuiLayer)
		{
			m_ImGuiLayer = ImGuiLayer::create();
			pushOverlay(m_ImGuiLayer);
		}
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

	void Application::_renderImGui()
	{
		m_ImGuiLayer->begin();

		for (const auto layer: m_layerStack)
		{
			layer->onGUIRender();
		}
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

				Application *app = this;
				if (m_enableImGuiLayer)
				{
					Renderer::submit([app]() { app->_renderImGui(); });
					Renderer::submit([this]() { m_ImGuiLayer->end(); });
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

	void Application::pushOverlay(ILayer *overlay)
	{
		m_layerStack.pushOverlay(overlay);
		overlay->onInit();
	}

	void Application::popOverlay(ILayer *overlay)
	{
		m_layerStack.popOverlay(overlay);
		overlay->onDestroy();
	}
}
