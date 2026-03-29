/*!
 * @file application.hpp
 */
#pragma once

#include <vector>

#include "layer.hpp"
#include "window.hpp"

#include "toast_lib/events/window_event.hpp"

namespace toaster
{
	struct ApplicationCreateInfo
	{
		WindowCreateInfo windowCreateInfo;
	};

	class Application
	{
	public:
		Application(const ApplicationCreateInfo &p_create_info);
		~Application() noexcept;

		void run();
		void close() noexcept;

		[[nodiscard]] Window &getWindow() const noexcept;

	private:
		bool onWindowCloseEvent(WindowCloseEvent &p_event);
		bool onWindowResizeEvent(WindowResizeEvent &p_event);

		ApplicationCreateInfo m_createInfo;
		Window *              m_window{nullptr};

		std::vector<IAppLayer *> m_layers;

	protected:
		void addLayer(IAppLayer *p_layer);
		void removeLayer(IAppLayer *p_layer);

		void setBeginUIRenderCallback(const std::function<void()> &p_cb_begin_ui_render);
		void setEndUIRenderCallback(const std::function<void()> &p_cb_end_ui_render);

	private:
		std::function<void()> m_cbBeginUIRender{nullptr};
		std::function<void()> m_cbEndUIRender{nullptr};

		float32 m_deltaTime{0.0f};
		float32 m_lastFrameTime{0.0f};

		bool m_minimized{false};
		bool m_isRunning{true};

		friend class ViewportLayer;
	};
}
