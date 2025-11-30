#pragma once

#include "render_thread.hpp"
#include "core/core.hpp"
#include "window.hpp"
#include "events/window_event.hpp"
#include "memory/unique_ptr.hpp"

#include "misc/layer_stack.hpp"
#include "nvrhi/nvrhi.h"

namespace tst
{
	struct ApplicationSpecInfo
	{
		String applicationName = "Orbo";
	};

	class Application
	{
	public:
		Application(std::vector<String> args);

		~Application();

		void run();

		uint32 getCurrentFrameIndex() { return m_currentFrameIndex; }

		Window &            getMainWindow() { return *m_window; }
		static Application &getInstance() { return *s_instance; }

		static gpu::DeviceManager *getDeviceManager() { return getInstance().getMainWindow().getDeviceManager(); }
		static nvrhi::DeviceHandle getGraphicsDevice() { return getDeviceManager()->getDevice(); }

		void pushLayer(ILayer *layer);
		void popLayer(ILayer *layer);

	protected:
		LayerStack m_layerStack;

	private:
		void _processEvents();

		void _onEvent(Event &event);
		bool _onWindowResize(WindowResizedEvent &event);
		bool _onWindowMinimize(WindowMinimizedEvent &event);
		bool _onWindowClose(WindowClosedEvent &event);

		ApplicationSpecInfo m_specInfo;

		UniquePtr<Window> m_window;

		RenderThread m_renderThread;

		bool m_isRunning = true;
		bool m_minimized = false;

		float32  m_deltaTime         = 0.0f;
		uint32_t m_currentFrameIndex = 0;

		static inline Application *s_instance = nullptr;
	};
}
