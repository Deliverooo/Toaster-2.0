#pragma once

#include "toast_kernel/application.hpp"
#include "toast_lib/ptr.hpp"

#include "editor_camera.hpp"
#include "panels/scene_hierarchy_panel.hpp"
#include "toast_lib/events/key_event.hpp"
#include "toast_lib/events/window_event.hpp"

#include <imgui.h>

namespace ig = ImGui;

namespace toaster
{
	namespace gpu
	{
		class VKPipeline;
		class VKRenderPass;
		class VKMaterial;
		class VKTexture2D;
		class VKRawImage;
		class VKUniformBuffer;
		class VKUniformBufferPFF;
	}

	class Renderer2D;
	class SceneRenderer;
	class Scene;

	class EditorLayer final : public IAppLayer
	{
	public:
		explicit EditorLayer(Application *p_app);

		virtual auto onInit() -> void override;
		virtual auto onDestroy() -> void override;
		virtual auto onUpdate(float32 p_dt) -> void override;
		virtual auto onEvent(Event &p_event) -> void override;

		virtual auto onUIRender() -> void override;

	private:
		auto _onWindowFileDropEvent(WindowFileDropEvent &p_event) -> bool;
		auto _onKeyPressEvent(KeyPressEvent &p_event) -> bool;

		gpu::VKLogicalDevice *m_device{nullptr};

		uint32 m_windowWidth{0u};
		uint32 m_windowHeight{0u};

		float32 m_time{0.0f};

		RefPtr<gpu::VKPipeline>   m_fullscreenPipeline{nullptr};
		RefPtr<gpu::VKRenderPass> m_fullscreenPass{nullptr};
		RefPtr<gpu::VKMaterial>   m_fullscreenMaterial{nullptr};

		RefPtr<Scene>                  m_scene{nullptr};
		UniquePtr<SceneHierarchyPanel> m_sceneHierarchyPanel{nullptr};
		RefPtr<SceneRenderer>          m_sceneRenderer{nullptr};

		RefPtr<Renderer2D> m_renderer2D{nullptr};

		EditorCamera m_editorCamera;

		int32 m_gizmoType{-1}; // Translate, rotate or scale
		int32 m_gizmoMode{0};  // 0 For local, 1 for world space

		bool m_canOperateCamera{true};
	};
}
