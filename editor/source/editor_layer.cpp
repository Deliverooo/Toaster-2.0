#include "editor_layer.hpp"

#include "toaster/toast_kernel/application.hpp"
#include "toaster/toast_kernel/input.hpp"
#include "toaster/toast_lib/io/file_stream.hpp"
#include "toaster/toast_render/globals.hpp"
#include "toaster/toast_render/renderer.hpp"

#include "toaster/toast_scene/components.hpp"

#include "editor_application.hpp"

#include <imgui.h>

#include "toaster/toast_lib/logging.hpp"
namespace ig = ImGui;

namespace toaster
{
	EditorLayer::EditorLayer(Application *p_app) : IAppLayer(p_app), m_cameraController(1920.0f / 1080.0f, true)
	{
	}

	void EditorLayer::onInit()
	{
		m_scene = make_reference<Scene>();

		m_sceneHierarchyPanel = make_reference<SceneHierarchyPanel>(m_scene);

		m_texture   = gpu::ITexture2D::create("resources/textures/Orbo_02.png");
		m_peeberTex = gpu::ITexture2D::create("resources/textures/peeber.png");

		gpu::FramebufferCreateInfo framebuffer_create_info{};
		framebuffer_create_info.width  = 1920;
		framebuffer_create_info.height = 1080;
		m_framebuffer                  = gpu::IFramebuffer::create(framebuffer_create_info);

		Renderer2DCreateInfo renderer_2d_create_info;
		renderer_2d_create_info.maxQuads = 1u;
		m_renderer2d                     = make_reference<Renderer2D>(renderer_2d_create_info);

		{
			Entity entity = m_scene->createEntity();

			auto &tc                = entity.getComponent<TransformComponent>();
			tc.transform            = glm::translate(glm::scale(glm::mat4{1.0f}, {1.0f, 1.0f, 1.0f}), {0.0f, 0.0f, -0.1f});
			auto &[colour, texture] = entity.addComponent<SpriteRendererComponent>();
			colour                  = {1.0f, 1.0f, 1.0f, 1.0f};
			texture                 = m_texture;
		}

		{
			m_cameraEntity = m_scene->createEntity(u8"オルボ　ステトソン");
			auto &cc       = m_cameraEntity.addComponent<CameraComponent>();

			class TestScript : public ScriptableEntity
			{
			public:
				void onCreate() override
				{
				}

				void onUpdate(float32 p_dt) override
				{
				}

				void onDestroy() override
				{
				}

			private:
			};

			m_cameraEntity.addComponent<NativeScriptComponent>().bind<TestScript>();
		}
	}

	void EditorLayer::onDestroy()
	{
	}

	void EditorLayer::onUpdate(const float32 p_dt)
	{
		m_time += p_dt;

		if (const auto &[width, height] = m_framebuffer->getCreateInfo();
			m_viewportSize.x > 0.0f && m_viewportSize.y > 0.0f && (static_cast<float32>(width) != m_viewportSize.x || static_cast<float32>(height) != m_viewportSize.y))
		{
			m_framebuffer->resize(static_cast<uint32>(m_viewportSize.x), static_cast<uint32>(m_viewportSize.y));
			m_cameraController.onResize(m_viewportSize.x, m_viewportSize.y);
			m_scene->setViewportSize(static_cast<uint32>(m_viewportSize.x), static_cast<uint32>(m_viewportSize.y));
			RenderCommand::setViewport({0.0f, 0.0f, m_viewportSize});
		}

		if (m_viewportFocused)
			m_cameraController.onUpdate(p_dt);

		m_framebuffer->bind();
		RenderCommand::clearColour({0.2f, 0.3f, 0.3f, 1.0f});
		RenderCommand::clear();

		m_scene->onUpdate(p_dt);
		m_scene->onRender(m_renderer2d, p_dt);

		m_framebuffer->unbind();
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

		auto &camera = m_cameraEntity.getComponent<CameraComponent>().camera;
		ig::DragFloat3("Camera Transform", &m_cameraEntity.getComponent<TransformComponent>().transform[3].x);

		ig::End(); // Settings

		m_sceneHierarchyPanel->onUIRender();

		ig::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ig::Begin("Viewport");

		m_viewportFocused = ig::IsWindowFocused();
		m_viewportHovered = ig::IsWindowHovered();
		((EditorApplication &) __super::getApp()).setBlockUIEvents(!m_viewportFocused || !m_viewportHovered);

		auto size      = ig::GetContentRegionAvail();
		m_viewportSize = {size.x, size.y};

		ig::Image(m_framebuffer->getColourAttachmentID(), size, ImVec2(0, 1), ImVec2(1, 0));

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
