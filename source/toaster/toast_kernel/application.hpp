/*!
 * @file application.hpp
 */
#pragma once

#include <vector>

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
		void close() noexcept;

		[[nodiscard]] Window &getWindow() const noexcept;

	private:
		bool onWindowClose(WindowCloseEvent &e);
		bool onWindowResize(WindowResizeEvent &e);

		Window *m_window{nullptr};

		std::vector<IAppLayer *> m_layers;

	protected:
		void addLayer(IAppLayer *layer);
		void removeLayer(IAppLayer *layer);

	private:
		float32 m_deltaTime{0.0f};
		float32 m_lastFrameTime{0.0f};

		bool m_minimized{false};
		bool m_isRunning{true};

		friend class ViewportLayer;
	};
}
