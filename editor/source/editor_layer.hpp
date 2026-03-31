#pragma once

#include "editor_camera.hpp"
#include "panels/scene_hierarchy_panel.hpp"
#include "toaster/toast_gpu/framebuffer.hpp"
#include "toaster/toast_kernel/layer.hpp"
#include "toaster/toast_render/renderer_2d.hpp"

#include "toaster/toast_gpu/shader.hpp"
#include "toaster/toast_gpu/texture.hpp"
#include "toaster/toast_kernel/ortho_camera_controller.hpp"

#include "toaster/toast_lib/events/key_event.hpp"
#include "toaster/toast_lib/events/mouse_event.hpp"
#include "toaster/toast_lib/events/window_event.hpp"

#include "toaster/toast_scene/entity.hpp"
#include "toaster/toast_scene/scene.hpp"

#include <array>

namespace toaster
{
	class EditorLayer : public IAppLayer
	{
	public:
		EditorLayer(Application *p_app);

		void onInit() override;
		void onDestroy() override;
		void onUpdate(float32 p_dt) override;
		void onEvent(Event &p_event) override;

		void onUIRender() override;

	private:
		void newScene();
		void saveScene();
		void openScene();
		bool onKeyPressEvent(KeyPressEvent &p_event);
		bool onMouseButtonPressEvent(MouseButtonPressEvent &p_event);

		RefPtr<Scene> m_scene;

		RefPtr<SceneHierarchyPanel> m_sceneHierarchyPanel;

		RefPtr<Renderer2D> m_renderer2d;

		RefPtr<gpu::IFramebuffer> m_framebuffer;

		EditorCamera m_editorCamera;

		String                   m_initialWindowTitle;
		glm::vec2                m_viewportSize{0.0f, 0.0f};
		std::array<glm::vec2, 2> m_viewportBounds{};

		int32 m_gizmoType{-1}; // Translate, rotate or scale
		int32 m_gizmoMode{0};  // 0 For local, 1 for world space

		Entity m_hoveredEntity{};

		volatile bool m_viewportFocused{false};
		volatile bool m_viewportHovered{false};
	};
}
