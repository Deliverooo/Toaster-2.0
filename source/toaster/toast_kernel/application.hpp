/*!
 * @file application.hpp
 */
#pragma once

#include "layer.hpp"
#include "window.hpp"

namespace toaster
{
	class WindowCloseEvent;
	class WindowResizeEvent;

	struct ApplicationCreateInfo
	{
		WindowCreateInfo windowCreateInfo{};
	};

	class Application
	{
	public:
		Application(const ApplicationCreateInfo &p_create_info);
		~Application() noexcept;

		auto run() -> void;
		auto close() noexcept -> void;

		[[nodiscard]] auto getWindow() const noexcept -> Window &;

	private:
		auto onWindowCloseEvent(WindowCloseEvent &p_event) -> bool;
		auto onWindowResizeEvent(WindowResizeEvent &p_event) -> bool;

		ApplicationCreateInfo m_createInfo{};
		Window *              m_window{nullptr};

		std::vector<IAppLayer *> m_layers;

		std::function<void()> m_cbBeginUIRender{nullptr};
		std::function<void()> m_cbEndUIRender{nullptr};

		float32 m_deltaTime{0.0f};
		float32 m_lastFrameTime{0.0f};

		bool m_minimized{false};
		bool m_isRunning{true};

	protected:
		auto addLayer(IAppLayer *p_layer) -> void;
		auto removeLayer(IAppLayer *p_layer) -> void;

		auto setBeginUIRenderCallback(const std::function<void()> &p_cb_begin_ui_render) -> void;
		auto setEndUIRenderCallback(const std::function<void()> &p_cb_end_ui_render) -> void;
	};
}
