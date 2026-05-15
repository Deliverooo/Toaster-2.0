/*!
 * @file application.hpp
 */
#pragma once

#include "layer.hpp"
#include "window.hpp"

#include <unordered_map>

#include "toast_lib/ptr.hpp"

namespace toaster
{
	class WindowCloseEvent;
	class WindowResizeEvent;

	namespace render
	{
		class Globals;
		class RenderContext;
	}

	struct TST_API ApplicationCreateInfo
	{
		WindowCreateInfo windowCreateInfo{};
	};

	using CommandLineArgMap = std::unordered_map<String, String>;

	class TST_API Application
	{
	public:
		Application(const ApplicationCreateInfo &p_create_info, const CommandLineArgMap &p_command_line_args);
		~Application() noexcept;

		auto run() -> void;
		auto close() noexcept -> void;

		[[nodiscard]] auto getWindow() const noexcept -> Window &;
		[[nodiscard]] auto getRenderContext() const noexcept -> render::RenderContext *;
		[[nodiscard]] auto getCommandLineArgs() const noexcept -> const CommandLineArgMap &;

	private:
		auto onWindowCloseEvent(WindowCloseEvent &p_event) -> bool;
		auto onWindowResizeEvent(WindowResizeEvent &p_event) -> bool;

		ApplicationCreateInfo m_createInfo{};

		CommandLineArgMap m_commandLineArgs;

		OwningPtr<render::RenderContext> m_renderContext{nullptr};
		OwningPtr<Window>                m_window{nullptr};

		std::vector<OwningPtr<IAppLayer> > m_layers;

		std::function<void()> m_cbBeginUIRender{nullptr};
		std::function<void()> m_cbEndUIRender{nullptr};

		float32 m_deltaTime{0.0f};
		float32 m_lastFrameTime{0.0f};

		bool m_minimized{false};
		bool m_isRunning{true};

		friend class IAppLayer;

	protected:
		auto addLayer(IAppLayer *p_layer) -> void;
		auto removeLayer(IAppLayer *p_layer) -> void;

		auto setBeginUIRenderCallback(const std::function<void()> &p_cb_begin_ui_render) -> void;
		auto setEndUIRenderCallback(const std::function<void()> &p_cb_end_ui_render) -> void;
	};
}
