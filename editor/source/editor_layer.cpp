#include "editor_layer.hpp"

#include "toaster/toast_kernel/application.hpp"
#include "toaster/toast_kernel/input.hpp"
#include "toaster/toast_lib/io/file_stream.hpp"
#include "toaster/toast_render/globals.hpp"
#include "toaster/toast_render/renderer.hpp"

#include <imgui.h>

#include "imgui_internal.h"

namespace toaster
{
	EditorLayer::EditorLayer(Application *p_app) : IAppLayer(p_app), m_cameraController(16.0f / 9.0f, true)
	{
	}

	void EditorLayer::onInit()
	{
		io::filesystem::setWorkingDirectory("../../../"); // The main Toaster dir (where the resource folder is)
		m_texture   = gpu::Texture2D::create("resources/textures/Orbo_02.png");
		m_peeberTex = gpu::Texture2D::create("resources/textures/teto.png");

		Renderer2DCreateInfo renderer_2d_create_info;
		renderer_2d_create_info.maxQuads = 1u;
		m_renderer2d                     = make_reference<Renderer2D>(renderer_2d_create_info);
	}

	void EditorLayer::onDestroy()
	{
	}

	void EditorLayer::onUpdate(const float32 p_dt)
	{
		m_time += p_dt;

		RenderCommand::clearColour({0.2f, 0.3f, 0.3f, 1.0f});
		RenderCommand::clear();

		m_cameraController.onUpdate(p_dt);

		m_renderer2d->begin(m_cameraController.getCamera().getViewMatrix(), m_cameraController.getCamera().getProjectionMatrix());

		m_renderer2d->submitQuad({2.0f, 0.0f, -0.1f}, {1.0f, 1.0f}, m_texture);
		m_renderer2d->submitQuad({0.0f, 1.0f, -0.1f}, {1.0f, 1.0f}, m_peeberTex);
		m_renderer2d->submitQuad({1.0f, 0.5f, -0.1f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f});

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
			const ImGuiViewport *viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
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
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		ImGui::Begin("DockSpace Demo", &p_open, window_flags);

		if (!opt_padding)
			ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// Submit the DockSpace
		ImGuiIO &   io        = ImGui::GetIO();
		ImGuiStyle &style     = ImGui::GetStyle();
		style.WindowMinSize.x = 300.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		style.WindowMinSize.x = 30.0f;
		#pragma endregion

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Quit", "Ctrl+Q"))
					__super::getApp().close();

				ImGui::Separator();
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		ImGui::Begin("Settings");
		ImGui::Text("Something...");
		ImGui::End(); // Settings

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
		ImGui::Begin("Viewport");

		ImGui::Image((ImTextureID) m_peeberTex->getID(), ImVec2(static_cast<float32>(m_peeberTex->getWidth()), static_cast<float32>(m_peeberTex->getHeight())),
					 ImVec2(0, 1), ImVec2(1, 0));

		ImGui::End(); // Viewport
		ImGui::PopStyleVar();

		ImGui::End(); // DockSpace Demo
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
		RenderCommand::setViewport({0, 0, p_event.getWidth(), p_event.getHeight()});

		return false;
	}
}
