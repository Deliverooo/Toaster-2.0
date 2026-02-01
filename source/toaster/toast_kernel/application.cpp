#include "application.hpp"

#include "io/filesystem.hpp"
#include "io/file_stream.hpp"

#include "logging.hpp"

#include <openglhpp/opengl.hpp>
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
			dispatcher.dispatch<WindowClosedEvent>([this](WindowClosedEvent &event) { return onWindowClose(event); });
			dispatcher.dispatch<WindowResizedEvent>([this](WindowResizedEvent &event) { return onWindowResize(event); });

			for (auto it = m_layers.rbegin(); it != m_layers.rend(); ++it)
			{
				if (e.isHandled())
					break;
				(*it)->onEvent(e);
			}
		});

		gpu::Globals::init();

		input::setCurrentWindowContext(m_window->getNativeWindow());
	}

	Application::~Application() noexcept
	{
		for (const IAppLayer *layer: m_layers)
		{
			delete layer;
			layer = nullptr;
		}
		m_layers.clear();

		gpu::Globals::shutdown();

		delete m_window;

		Window::shutdownWindowingAPI();
	}

	void Application::run()
	{
		while (m_isRunning)
		{
			const auto startTime = static_cast<float32>(glfwGetTime());

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

			const auto endTime = static_cast<float32>(glfwGetTime());
			m_deltaTime        = endTime - startTime;
		}
	}

	void Application::close() noexcept
	{
		m_isRunning = false;
	}

	Window &Application::getWindow() noexcept
	{
		return *m_window;
	}

	bool Application::onWindowClose(WindowClosedEvent &e)
	{
		m_isRunning = false;
		return true;
	}

	bool Application::onWindowResize(WindowResizedEvent &e)
	{
		if (e.getWidth() == 0 || e.getHeight() == 0)
		{
			m_minimized = true;
			return false;
		}
		m_minimized = false;
		return false;
	}
}
