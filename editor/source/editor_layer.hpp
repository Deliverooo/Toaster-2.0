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

#include "toast_gpu/vk/vk_gpu_context.hpp"
#include "toast_gpu/vk/vk_vertex_buffer.hpp"
#include "toast_gpu/vk/vk_index_buffer.hpp"
#include "toast_gpu/vk/vk_shader.hpp"
#include "toast_gpu/vk/vk_mesh.hpp"
#include "toast_gpu/vk/vk_pipeline.hpp"
#include "toast_gpu/vk/vk_texture.hpp"
#include "toast_gpu/vk/vk_uniform_buffer.hpp"

namespace toaster
{
	class EditorLayer final : public IAppLayer
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

		float32 m_time{0.0f};

		uint32 m_windowWidth{0u};
		uint32 m_windowHeight{0u};

		#pragma region fullscreen pass
		gpu::VertexBufferLayout   m_compositeVertexBufferLayout;
		RefPtr<gpu::VKShader>     m_compositeShader{nullptr};
		RefPtr<gpu::VKPipeline>   m_compositePipeline{nullptr};
		RefPtr<gpu::VKRenderPass> m_fullscreenPass{nullptr};

		RefPtr<gpu::VKMaterial> m_fullscreenMaterial{nullptr};

		RefPtr<gpu::VKVertexBuffer> m_fullscreenQuadVertexBuffer{nullptr};
		RefPtr<gpu::VKIndexBuffer>  m_fullscreenQuadIndexBuffer{nullptr};

		struct FullscreenQuadVertex
		{
			glm::vec3 positon;
			glm::vec2 texCoord;
		};

		std::vector<FullscreenQuadVertex> m_fullscreenQuadVertices;
		std::vector<uint16>               m_fullscreenQuadIndices;

		RefPtr<gpu::VKImage2D> m_colourAttachmentImage{nullptr};
		RefPtr<gpu::VKImage2D> m_depthAttachmentImage{nullptr};

		// vk::raii::DescriptorPool             m_descriptorPool{nullptr};
		// std::vector<vk::raii::DescriptorSet> m_compositeDescriptorSets;
		#pragma  endregion

		RefPtr<Scene>               m_scene;
		RefPtr<SceneHierarchyPanel> m_sceneHierarchyPanel;

		RefPtr<Renderer2D> m_renderer2D;

		EditorCamera m_editorCamera;

		String                   m_initialWindowTitle;
		std::array<glm::vec2, 2> m_viewportBounds{};
		glm::vec2                m_viewportSize{0.0f};

		int32 m_gizmoType{-1}; // Translate, rotate or scale
		int32 m_gizmoMode{0};  // 0 For local, 1 for world space

		Entity m_hoveredEntity{};

		volatile bool m_viewportFocused{false};
		volatile bool m_viewportHovered{false};
	};
}
