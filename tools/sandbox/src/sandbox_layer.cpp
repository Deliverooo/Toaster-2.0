#include "sandbox_layer.hpp"

#include "platform/application.hpp"

#include "events/key_event.hpp"
#include "key_codes.hpp"
#include "events/mouse_event.hpp"
#include "events/window_event.hpp"
#include "nvrhi/utils.h"
#include "platform/input.hpp"
#include "renderer/renderer.hpp"
#include "renderer/renderer_2d.hpp"
#include "ui/ui_core.hpp"
#include "gpu/framebuffer.hpp"

namespace tst
{
	SandboxLayer::SandboxLayer()
	{
	}

	SandboxLayer::~SandboxLayer()
	{
	}

	void SandboxLayer::onInit()
	{
		Renderer2DSpecInfo spec{};
		m_renderer2d = make_reference<Renderer2D>(spec);

		tsm::vec2ui window_size = Application::getInstance().getMainWindow().getSize();

		FramebufferSpecInfo framebuffer_spec{};
		framebuffer_spec.Width            = window_size.x;
		framebuffer_spec.Height           = window_size.y;
		framebuffer_spec.ClearColorOnLoad = false;
		framebuffer_spec.ClearDepthOnLoad = false;
		framebuffer_spec.Attachments      = {ImageFormat::RGBA32F, ImageFormat::DEPTH32FSTENCIL8UINT};

		m_framebuffer = make_reference<Framebuffer>(framebuffer_spec);

		m_texture = make_reference<Texture2D>(TextureSpecInfo{}, "res/textures/RGB.png");
	}

	void SandboxLayer::onDestroy()
	{
	}

	void SandboxLayer::onUpdate(float32 dt)
	{
		tsm::vec2ui window_size = Application::getInstance().getMainWindow().getSize();

		auto      view       = glm::mat4(1.0f);
		glm::mat4 projection = glm::ortho(glm::radians(90.0f), window_size.aspect(), 0.1f, 10.0f);

		m_renderer2d->SetTargetFramebuffer(m_framebuffer);
		m_renderer2d->BeginScene(view * projection, view);

		glm::mat4 quad_transform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
		m_renderer2d->DrawQuad(quad_transform, glm::vec4{1.0f, 1.0f, 0.0f, 1.0f});

		m_renderer2d->EndScene();
	}

	void SandboxLayer::onEvent(Event &event)
	{
		// Handle input events
	}

	void SandboxLayer::onGUIRender()
	{
		// ImGui + Dockspace Setup ------------------------------------------------------------------------------
		ImGuiIO &   io        = ImGui::GetIO();
		ImGuiStyle &style     = ImGui::GetStyle();
		auto        boldFont  = io.Fonts->Fonts[0];
		auto        largeFont = io.Fonts->Fonts[1];

		io.ConfigWindowsResizeFromEdges = io.BackendFlags & ImGuiBackendFlags_HasMouseCursors;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;

		ImGuiViewport *viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		auto *window      = static_cast<GLFWwindow *>(Application::getInstance().getMainWindow().getNativeWindow());
		bool  isMaximized = (bool) glfwGetWindowAttrib(window, GLFW_MAXIMIZED);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, isMaximized ? ImVec2(6.0f, 6.0f) : ImVec2(1.0f, 1.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 3.0f);

		ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4{0.0f, 0.0f, 0.0f, 0.0f});
		ImGui::Begin("DockSpace Demo", nullptr, window_flags);
		ImGui::PopStyleColor(); // MenuBarBg
		ImGui::PopStyleVar(2);
		ImGui::PopStyleVar(2);

		// Dockspace
		float minWinSizeX     = style.WindowMinSize.x;
		style.WindowMinSize.x = 370.0f;
		ImGui::DockSpace(ImGui::GetID("MyDockspace"));
		style.WindowMinSize.x = minWinSizeX;

		// Editor Panel ------------------------------------------------------------------------------
		//ImGui::ShowDemoWindow();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		if (ImGui::Begin("Viewport"))
		{
			tsm::vec2ui window_size = Application::getInstance().getMainWindow().getSize();

			ui::image(m_texture, {static_cast<float>(window_size.x), static_cast<float>(window_size.y)});
		}

		ImGui::End();
		ImGui::PopStyleVar();

		ImGui::End();
	}
}
