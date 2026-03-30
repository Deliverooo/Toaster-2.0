#include "editor_layer.hpp"

#include "toaster/toast_kernel/application.hpp"
#include "toaster/toast_kernel/input.hpp"
#include "toaster/toast_lib/io/file_stream.hpp"
#include "toaster/toast_render/globals.hpp"
#include "toaster/toast_render/renderer.hpp"

#include "toaster/toast_scene/components.hpp"
#include "toaster/toast_scene/scene_serializer.hpp"

#include "editor_application.hpp"

#include "toast_lib/os/file_dialog.hpp"

#include "ui/ui_utils.hpp"

#include "toaster/toast_lib/logging.hpp"

#include <ImGuizmo.h>
namespace ig = ImGui;
namespace igz = ImGuizmo;

#include "toast_lib/math/math_matrix.hpp"

namespace toaster
{
	EditorLayer::EditorLayer(Application *p_app) : IAppLayer(p_app), m_editorCamera(90.0f, 1.7776f, 0.1f, 1000.0f)
	{
	}

	void EditorLayer::onInit()
	{
		m_scene = make_reference<Scene>();

		m_sceneHierarchyPanel = make_reference<SceneHierarchyPanel>(m_scene);

		m_texture   = gpu::ITexture2D::create("resources/textures/Orbo_02.png");
		m_peeberTex = gpu::ITexture2D::create("resources/textures/peeber.png");

		gpu::FramebufferCreateInfo framebuffer_create_info{};
		framebuffer_create_info.width       = 1920;
		framebuffer_create_info.height      = 1080;
		framebuffer_create_info.samples     = 1u;
		framebuffer_create_info.attachments = {gpu::EImageFormat::eRGBA32F, gpu::EImageFormat::eDepth32FStencil8UInt};

		m_framebuffer = gpu::IFramebuffer::create(framebuffer_create_info);

		Renderer2DCreateInfo renderer_2d_create_info;
		renderer_2d_create_info.maxQuads = 1000u;
		m_renderer2d                     = make_reference<Renderer2D>(renderer_2d_create_info);

		auto &app            = getApp();
		m_initialWindowTitle = app.getWindow().getTitle();
		app.getWindow().setTitle(m_initialWindowTitle + " -> " + m_scene->getName());
	}

	void EditorLayer::onDestroy()
	{
	}

	void EditorLayer::onUpdate(const float32 p_dt)
	{
		{
			auto   create_info = m_framebuffer->getCreateInfo();
			uint32 width       = create_info.width;
			uint32 height      = create_info.height;
			if (m_viewportSize.x > 0.0f && m_viewportSize.y > 0.0f && (
					static_cast<float32>(width) != m_viewportSize.x || static_cast<float32>(height) != m_viewportSize.y))
			{
				m_framebuffer->resize(static_cast<uint32>(m_viewportSize.x), static_cast<uint32>(m_viewportSize.y));
				m_editorCamera.setViewportSize(m_viewportSize.x, m_viewportSize.y);
				m_scene->setViewportSize(static_cast<uint32>(m_viewportSize.x), static_cast<uint32>(m_viewportSize.y));
				RenderCommand::setViewport({0.0f, 0.0f, m_viewportSize});
			}
		}
		if (m_viewportFocused)
		{
			m_editorCamera.onUpdate(p_dt);
		}

		m_framebuffer->bind();
		RenderCommand::clearColour({0.2f, 0.3f, 0.3f, 1.0f});
		RenderCommand::clear();

		m_scene->onUpdate(p_dt);
		m_scene->onRender(p_dt, m_renderer2d, m_editorCamera.getViewMatrix(), m_editorCamera.getProjectionMatrix());

		m_framebuffer->unbind();
	}

	void EditorLayer::onEvent(Event &p_event)
	{
		EventDispatcher eventDispatcher(p_event);
		eventDispatcher.dispatch<MouseMoveEvent>(TST_BIND_EVENT_FN(EditorLayer::onMouseMoveEvent));
		eventDispatcher.dispatch<WindowResizeEvent>(TST_BIND_EVENT_FN(EditorLayer::onWindowResizeEvent));
		eventDispatcher.dispatch<KeyPressEvent>(TST_BIND_EVENT_FN(EditorLayer::onKeyPressEvent));

		m_editorCamera.onEvent(p_event);
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
				if (ig::MenuItem("New", "Ctrl+N"))
				{
					newScene();
				}
				if (ig::MenuItem("Save", "Ctrl+S"))
				{
					saveScene();
				}
				if (ig::MenuItem("Open", "Ctrl+O"))
				{
					openScene();
				}
				if (ig::MenuItem("Quit", "Ctrl+Q"))
					getApp().close();

				ig::Separator();
				ig::EndMenu();
			}
			ig::EndMenuBar();
		}

		ig::Begin("Settings");

		ig::Text("%d", m_renderer2d->getStats().quadCount);

		ig::End(); // Settings

		m_sceneHierarchyPanel->onUIRender();

		{
			ui::ScopedStyle window_padding{ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f)};

			ig::Begin("Viewport");

			m_viewportFocused = ig::IsWindowFocused();
			m_viewportHovered = ig::IsWindowHovered();
			((EditorApplication &) getApp()).setBlockUIEvents(!m_viewportFocused && !m_viewportHovered);

			auto size      = ig::GetContentRegionAvail();
			m_viewportSize = {size.x, size.y};

			ig::Image(m_framebuffer->getColourAttachmentID(), size, ImVec2(0, 1), ImVec2(1, 0));

			Entity selected_entity = m_sceneHierarchyPanel->getSelectedEntity();
			if (selected_entity && m_gizmoType != -1)
			{
				igz::SetOrthographic(false);
				igz::SetDrawlist();
				igz::SetRect(ig::GetWindowPos().x, ig::GetWindowPos().y, ig::GetWindowWidth(), ig::GetWindowHeight());

				// auto camera_entity = m_scene->getMainCameraEntity();

				// if (camera_entity)
				// {
				const glm::mat4 &view = m_editorCamera.getViewMatrix();
				glm::mat4        proj = m_editorCamera.getProjectionMatrix();

				auto &tc = selected_entity.getComponent<TransformComponent>();

				glm::mat4 entity_transform = tc.getTransform();

				bool    snap_transform = input::isKeyDown(input::EKeyCode::eLeftControl);
				float32 snap_value{0.5f};

				if (m_gizmoType == igz::OPERATION::ROTATE)
					snap_value = 45.0f;

				const float32 snap_values[3] = {snap_value, snap_value, snap_value};

				if (igz::Manipulate(&view[0].x, &proj[0].x, static_cast<igz::OPERATION>(m_gizmoType), static_cast<igz::MODE>(m_gizmoMode), &entity_transform[0].x,
									nullptr, snap_transform ? snap_values : nullptr))
				{
					glm::vec3 translation;
					glm::quat rotation;
					glm::vec3 scale;

					tsm::decomposeTransform(entity_transform, translation, rotation, scale);

					tc.translation = translation;

					const glm::vec3 delta_rotation = glm::eulerAngles(rotation) - tc.rotation;
					tc.rotation                    += delta_rotation;
					tc.scale                       = scale;
				}
				// }
			}

			ig::End(); // Viewport
		}

		ig::End(); // DockSpace Demo
	}

	void EditorLayer::newScene()
	{
		m_scene = make_reference<Scene>();
		m_scene->setViewportSize(static_cast<uint32>(m_viewportSize.x), static_cast<uint32>(m_viewportSize.y));
		m_sceneHierarchyPanel->setScene(m_scene);
	}

	void EditorLayer::saveScene()
	{
		auto save_location = os::saveFileDialog({{"Toaster Scene", "tscene"}});
		if (!save_location.empty())
		{
			SceneSerializer ss{m_scene};
			ss.serialize(save_location);
		}
	}

	void EditorLayer::openScene()
	{
		auto scene_location = os::openFileDialog({{"Toaster Scene", "tscene"}});
		if (!scene_location.empty())
		{
			newScene();

			SceneSerializer ss{m_scene};
			ss.deserialize(scene_location);

			LOG_INFO("{}", scene_location.string());
			auto &app = getApp();
			app.getWindow().setTitle(m_initialWindowTitle + " -> " + m_scene->getName());
		}
	}

	bool EditorLayer::onKeyPressEvent(KeyPressEvent &p_event)
	{
		if (m_viewportFocused)
		{
			if (m_viewportHovered && !input::isMouseButtonDown(input::EMouseButton::eRight))
			{
				switch (p_event.getKeyCode())
				{
					case input::EKeyCode::eQ:
						m_gizmoType = -1;
						break;
					case input::EKeyCode::eW:
						m_gizmoType = igz::OPERATION::TRANSLATE;
						break;
					case input::EKeyCode::eE:
						m_gizmoType = igz::OPERATION::ROTATE;
						break;
					case input::EKeyCode::eR:
						m_gizmoType = igz::OPERATION::SCALE;
						break;
					case input::EKeyCode::eL:
					{
						// Switch between world and local space transforming for the gizmos
						if (input::isKeyDown(input::EKeyCode::eLeftAlt))
						{
							if (m_gizmoMode == igz::MODE::LOCAL)
								m_gizmoMode = igz::MODE::WORLD;
							else
								m_gizmoMode = igz::MODE::LOCAL;
						}
						break;
					}
					default: break;
				}
			}
		}

		if (p_event.getKeyCode() == input::EKeyCode::eEscape)
		{
			getApp().close();
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
