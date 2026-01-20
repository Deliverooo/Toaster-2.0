#pragma once

#include <vector>

#include "system_types.h"

#include <glm/glm.hpp>

#include "input.hpp"
#include "layer.hpp"
#include "window.hpp"

#include "events/window_event.hpp"

namespace toaster
{
	class Application
	{
	public:
		Application();
		~Application() noexcept;

		void run();

	private:
		bool onWindowClose(WindowClosedEvent &e);
		bool onWindowResize(WindowResizedEvent &e);

		Window *m_window{nullptr};

	protected:
		std::vector<IAppLayer *> m_layers;

	private:
		float32 m_deltaTime{0.0f};
		bool    m_minimized{false};
		bool    m_isRunning{true};

		friend class ViewportLayer;
	};
}
