#include "editor_layer.hpp"

#include "toaster/toast_kernel/application.hpp"
#include "toaster/toast_kernel/input.hpp"
#include "toaster/toast_lib/io/file_stream.hpp"
#include "toaster/toast_render/globals.hpp"
#include "toaster/toast_render/renderer.hpp"

#include "editor_application.hpp"

#include <imgui.h>
namespace ig = ImGui;

namespace toaster
{
	EditorLayer::EditorLayer(Application *p_app) : IAppLayer(p_app), m_cameraController(16.0f / 9.0f, true)
	{
	}

	void EditorLayer::onInit()
	{
		m_scene = make_reference<Scene>();

		m_texture   = gpu::Texture2D::create("resources/textures/Orbo_02.png");
		m_peeberTex = gpu::Texture2D::create("resources/textures/peeber.png");

		gpu::FramebufferCreateInfo framebuffer_create_info{};
		framebuffer_create_info.width  = 1920;
		framebuffer_create_info.height = 1080;
		m_framebuffer                  = gpu::Framebuffer::create(framebuffer_create_info);

		Renderer2DCreateInfo renderer_2d_create_info;
		renderer_2d_create_info.maxQuads          = 1u;
		renderer_2d_create_info.targetFramebuffer = m_framebuffer;
		m_renderer2d                              = make_reference<Renderer2D>(renderer_2d_create_info);

		Entity entity = m_scene->createEntity();

		auto &tc     = entity.getComponent<TransformComponent>();
		tc.transform = glm::translate(glm::mat4{1.0f}, glm::vec3(0.0f, 0.0f, -0.1f));
		auto &src    = entity.addComponent<SpriteRendererComponent>();
		src.colour   = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
		src.texture  = m_peeberTex;
	}

	void EditorLayer::onDestroy()
	{
	}

	void EditorLayer::onUpdate(const float32 p_dt)
	{
		m_time += p_dt;

		if (m_viewportFocused)
			m_cameraController.onUpdate(p_dt);

		m_framebuffer->bind();
		RenderCommand::clearColour({0.2f, 0.3f, 0.3f, 1.0f});
		RenderCommand::clear();

		m_renderer2d->begin(m_cameraController.getCamera().getViewMatrix(), m_cameraController.getCamera().getProjectionMatrix());
		m_scene->onUpdate(m_renderer2d, p_dt);

		m_renderer2d->submitQuad({2.0f, 0.0f, -0.1f}, {1.0f, 1.0f}, m_peeberTex);
		m_renderer2d->submitQuad({0.0f, 1.0f, -0.1f}, {1.0f, 1.0f}, m_peeberTex);
		m_renderer2d->submitQuad({1.0f, 0.5f, -0.1f}, {1.0f, 1.0f}, m_colour);

		m_renderer2d->end();
	}

	void EditorLayer::onEvent(Event &p_event)
	{
		EventDispatcher eventDispatcher(p_event);
		eventDispatcher.dispatch<MouseMoveEvent>(TST_BIND_EVENT_FN(EditorLayer::onMouseMoveEvent));
		eventDispatcher.dispatch<WindowResizeEvent>(TST_BIND_EVENT_FN(EditorLayer::onWindowResizeEvent));
		eventDispatcher.dispatch<KeyPressEvent>(TST_BIND_EVENT_FN(EditorLayer::onKeyPressEvent));

		m_cameraController.onEvent(p_event);
	}

	void EditorLayer::onUIRender()
	{
		#pragma region Setup Dockspace
		static bool               p_open          = true;
		static bool               opt_fullscreen  = true;
		static bool               opt_padding     = false;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			const ImGuiViewport *viewport = ig::GetMainViewport();
			ig::SetNextWindowPos(viewport->WorkPos);
			ig::SetNextWindowSize(viewport->WorkSize);
			ig::SetNextWindowViewport(viewport->ID);
			ig::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ig::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}
		else
		{
			dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
		}

		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		if (!opt_padding)
			ig::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		ig::Begin("DockSpace Demo", &p_open, window_flags);

		if (!opt_padding)
			ig::PopStyleVar(); // ImGuiStyleVar_WindowPadding

		if (opt_fullscreen)
			ig::PopStyleVar(2); // ImGuiStyleVar_WindowRounding ImGuiStyleVar_WindowBorderSize

		// Submit the DockSpace
		ImGuiIO &   io        = ig::GetIO();
		ImGuiStyle &style     = ig::GetStyle();
		style.WindowMinSize.x = 300.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ig::GetID("MyDockSpace");
			ig::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		style.WindowMinSize.x = 30.0f;
		#pragma endregion

		if (ig::BeginMenuBar())
		{
			if (ig::BeginMenu("File"))
			{
				if (ig::MenuItem("Quit", "Ctrl+Q"))
					__super::getApp().close();

				ig::Separator();
				ig::EndMenu();
			}
			ig::EndMenuBar();
		}

		ig::Begin("Settings");

		ig::Text("Orbo");

		ig::ColorEdit4("Col", &m_colour.x);

		ig::End(); // Settings

		ig::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ig::Begin("Viewport");

		m_viewportFocused = ig::IsWindowFocused();
		m_viewportHovered = ig::IsWindowHovered();
		((EditorApplication &) __super::getApp()).setBlockUIEvents(!m_viewportFocused || !m_viewportHovered);

		auto size = ig::GetContentRegionAvail();
		if (m_viewportSize != *reinterpret_cast<glm::vec2 *>(&size))
		{
			m_viewportSize = {size.x, size.y};
			m_framebuffer->resize(static_cast<uint32>(size.x), static_cast<uint32>(size.y));

			RenderCommand::setViewport({0, 0, m_viewportSize});

			m_cameraController.onResize(m_viewportSize.x, m_viewportSize.y);
		}

		ig::Image(m_framebuffer->getColourAttachmentID(), ImVec2(m_viewportSize.x, m_viewportSize.y), ImVec2(0, 1), ImVec2(1, 0));

		ig::End();         // Viewport
		ig::PopStyleVar(); // ImGuiStyleVar_WindowPadding

		ig::End(); // DockSpace Demo
	}

	bool EditorLayer::onKeyPressEvent(KeyPressEvent &p_event)
	{
		if (p_event.getKeyCode() == input::EKeyCode::eI)
		{
			if (input::getCursorMode() == input::ECursorMode::eDisabled)
			{
				input::setCursorMode(input::ECursorMode::eNormal);
			}
			else
			{
				input::setCursorMode(input::ECursorMode::eDisabled);
			}
		}

		if (p_event.getKeyCode() == input::EKeyCode::eEscape)
		{
			__super::getApp().close();
		}

		return false;
	}

	bool EditorLayer::onMouseMoveEvent(MouseMoveEvent &p_event)
	{
		// Stuff...
		return false;
	}

	bool EditorLayer::onWindowResizeEvent(WindowResizeEvent &p_event)
	{
		return false;
	}
}
