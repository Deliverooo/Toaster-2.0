#include "toast_kernel/application.hpp"

#include "toast_lib/events/window_event.hpp"

#include "toast_kernel/input.hpp"
#include "toast_render/render_context.hpp"

#include <GLFW/glfw3.h>

#include "toast_gpu/vk/vk_swapchain.hpp"
#include "toast_lib/os/terminal.hpp"

#include "toast_scene/scene.hpp"
#include "toast_scene/scene_renderer.hpp"

namespace toaster
{
	Application::Application(const ApplicationSpecInfo &p_spec_info, const CommandLineArgs *p_command_line_args) : m_specInfo(p_spec_info),
																												   m_commandLineArgs(p_command_line_args)
	{
		Window::initWindowingAPI();

		render::RenderContextSpecInfo render_context_spec_info{};
		render_context_spec_info.sdkDir             = m_specInfo.sdkDir / "bin";
		render_context_spec_info.instanceExtensions = Window::getRequiredInstanceExtensions();
		render_context_spec_info.printDebugInfo     = m_specInfo.printGPUDebugInfo;
		m_renderContext                             = new render::RenderContext{render_context_spec_info};

		TST_PERMA_ASSERT(m_renderContext);

		#pragma region create window
		m_window = new Window(m_renderContext, m_specInfo.windowSpecInfo);
		m_window->setEventCallback([this](Event &e)
		{
			EventDispatcher dispatcher{e};
			dispatcher.dispatch<WindowCloseEvent>(TST_BIND_EVENT_FN(Application::onWindowCloseEvent));
			dispatcher.dispatch<WindowResizeEvent>(TST_BIND_EVENT_FN(Application::onWindowResizeEvent));

			for (const auto &layer: m_layers)
			{
				if (e.isHandled())
					continue;
				layer->onEvent(e);
			}
		});

		m_window->getSwapchain()->setResizeUserDataPointer(this);
		m_window->getSwapchain()->setResizeCallback([](void *p_user_data, tsm::uint2 p_size) -> void
		{
			for (const auto app{static_cast<Application *>(p_user_data)}; const auto &layer: app->m_layers)
			{
				layer->onResize(p_size);
			}
		});

		#pragma endregion
	}

	Application::~Application() noexcept
	{
		for (auto &layer: m_layers)
		{
			layer->onDestroy();
			m_renderContext->gpuWaitIdle(); // Wait until the GPU is finished using all the layers' resources
			delete layer;
		}

		m_layers.clear();

		delete m_window;
		delete m_renderContext;
		Window::shutdownWindowingAPI();
	}

	auto Application::run() -> int32
	{
		for (auto &layer: m_layers)
			layer->onUIInit(m_onUIInitUserData);

		while (m_isRunning)
		{
			const auto startTime{static_cast<float32>(glfwGetTime())};
			m_deltaTime     = startTime - m_lastFrameTime;
			m_lastFrameTime = startTime;

			m_window->processEvents();
			m_window->beginFrame();

			m_renderContext->setCurrentCommandBuffer(&m_window->getSwapchain()->getCurrentCommandBuffer());
			m_renderContext->getDescriptorHeap()->bind();

			if (!m_minimized)
			{
				for (auto &layer: m_layers)
					layer->onUpdate(m_deltaTime);

				if (m_cbBeginUIRender)
					m_cbBeginUIRender();

				for (auto &layer: m_layers)
					layer->onUIRender();

				if (m_cbEndUIRender)
					m_cbEndUIRender();
			}
			m_window->endFrame();
		}

		return 0;
	}

	auto Application::close() noexcept -> void
	{
		m_isRunning = false;
	}

	auto Application::createScene(const String &p_name) const -> UniquePtr<Scene>
	{
		return toaster::make_unique<Scene>(m_renderContext, nullptr, p_name);
	}

	auto Application::createSceneRenderer(Scene *p_scene) const -> UniquePtr<SceneRenderer>
	{
		SceneRendererSpecInfo scene_renderer_spec_info{};
		scene_renderer_spec_info.viewportSize = m_window->getRenderAreaSize();
		return make_unique<SceneRenderer>(p_scene, scene_renderer_spec_info);
	}

	auto Application::onWindowCloseEvent([[maybe_unused]] WindowCloseEvent &p_event) -> bool
	{
		m_isRunning = false;
		return true;
	}

	auto Application::onWindowResizeEvent(WindowResizeEvent &p_event) -> bool
	{
		const uint32 width{p_event.getWidth()};
		const uint32 height{p_event.getHeight()};

		if (width == 0 || height == 0)
		{
			m_minimized = true;
			return false;
		}

		m_minimized = false;
		return false;
	}

	auto Application::setOnUIInitUserData(void *p_user_data) -> void
	{
		m_onUIInitUserData = p_user_data;
	}

	auto Application::setBeginUIRenderCallback(const std::function<void()> &p_cb_begin_ui_render) -> void
	{
		m_cbBeginUIRender = p_cb_begin_ui_render;
	}

	auto Application::setEndUIRenderCallback(const std::function<void()> &p_cb_end_ui_render) -> void
	{
		m_cbEndUIRender = p_cb_end_ui_render;
	}
}
