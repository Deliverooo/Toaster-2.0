#pragma once

#include "toast_kernel/application.hpp"
#include "toast_lib/ptr.hpp"

#include <glm/glm.hpp>

#include "editor_camera.hpp"
#include "panels/scene_hierarchy_panel.hpp"
#include "toast_lib/events/window_event.hpp"

namespace toaster
{
	namespace gpu
	{
		class VKPipeline;
		class VKRenderPass;
		class VKMaterial;
		class VKTexture2D;
		class VKImage2D;
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
		auto   _onWindowFileDropEvent(WindowFileDropEvent &p_event) -> bool;

		uint32 m_viewportWidth{0u};
		uint32 m_viewportHeight{0u};

		// RefPtr<gpu::VKTexture2D> m_finalColourTexture{nullptr};
		// RefPtr<gpu::VKImage2D>   m_finalDepthImage{nullptr};

		RefPtr<gpu::VKPipeline>   m_fullscreenPipeline{nullptr};
		RefPtr<gpu::VKRenderPass> m_fullscreenPass{nullptr};
		RefPtr<gpu::VKMaterial>   m_fullscreenMaterial{nullptr};

		RefPtr<gpu::VKTexture2D> m_texture{nullptr};
		RefPtr<gpu::VKTexture2D> m_texture2{nullptr};

		float32 m_time{0.0f};

		struct FrameDataUB
		{
			glm::vec2 res{1.0f};
			float32   time{0.0f};
		};

		RefPtr<gpu::VKUniformBufferPFF> m_frameDataUBOs{nullptr};

		RefPtr<Scene>                  m_scene{nullptr};
		UniquePtr<SceneHierarchyPanel> m_sceneHierarchyPanel{nullptr};
		RefPtr<SceneRenderer>          m_sceneRenderer{nullptr};

		VkDescriptorSet m_imguiSceneRendererDescriptorSet{nullptr};

		RefPtr<Renderer2D> m_renderer2D{nullptr};

		EditorCamera m_editorCamera;

		bool m_canOperateCamera{true};
	};
}
