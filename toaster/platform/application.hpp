#pragma once

#include <filesystem>

#include "render_thread.hpp"
#include "core/core.hpp"
#include "window.hpp"
#include "events/window_event.hpp"
#include "core/unique_ptr.hpp"

#include "core/layer_stack.hpp"
#include "nvrhi/nvrhi.h"
#include "renderer/renderer_config_info.hpp"
#include "ui/imgui_layer.hpp"

namespace tst
{
	struct ApplicationSpecInfo
	{
		std::string Name            = "Toaster";
		uint32_t    WindowWidth     = 1600, WindowHeight = 900;
		bool        WindowDecorated = false;
		bool        Fullscreen      = false;
		bool        VSync           = true;
		std::string WorkingDirectory;
		bool        StartMaximized = true;
		bool        Resizable      = true;
		bool        EnableImGui    = true;

		RendererConfigInfo    RenderConfig;
		EThreadingPolicy      CoreThreadingPolicy = EThreadingPolicy::eMultiThreaded;
		std::filesystem::path IconPath;
	};

	class Application
	{
	public:
		Application(const ApplicationSpecInfo & spec_info);

		~Application();

		void run();

		uint32 getCurrentFrameIndex() { return m_currentFrameIndex; }

		Window &            getMainWindow() { return *m_window; }
		static Application &getInstance() { return *s_instance; }

		static DeviceManager *     getDeviceManager() { return getInstance().getMainWindow().getDeviceManager(); }
		static nvrhi::DeviceHandle getGraphicsDevice() { return getDeviceManager()->GetDevice(); }

		ImGuiLayer *getImGuiLayer() { return m_ImGuiLayer; }

		RenderThread &getRenderThread() { return m_renderThread; }

		void pushLayer(ILayer *layer);
		void popLayer(ILayer *layer);
		void pushOverlay(ILayer *overlay);
		void popOverlay(ILayer *overlay);

	protected:
		LayerStack m_layerStack;

	private:
		void _renderImGui();
		void _processEvents();

		void _onEvent(Event &event);
		bool _onWindowResize(WindowResizedEvent &event);
		bool _onWindowMinimize(WindowMinimizedEvent &event);
		bool _onWindowClose(WindowClosedEvent &event);

		ApplicationSpecInfo m_specInfo;

		UniquePtr<Window> m_window;
		ImGuiLayer *      m_ImGuiLayer;

		RenderThread m_renderThread;

		bool m_isRunning        = true;
		bool m_minimized        = false;
		bool m_enableImGuiLayer = true;

		float32  m_deltaTime         = 0.0f;
		uint32_t m_currentFrameIndex = 0;

		static inline Application *s_instance = nullptr;
	};
}
