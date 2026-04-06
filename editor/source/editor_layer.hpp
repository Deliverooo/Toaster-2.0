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

		void                        _createRenderTargetResources(uint32 p_width, uint32 p_height);
		RefPtr<Scene>               m_scene;
		RefPtr<SceneHierarchyPanel> m_sceneHierarchyPanel;

		RefPtr<Renderer2D> m_renderer2d;

		vk::raii::Image        m_renderTargetImage{nullptr};
		vk::raii::DeviceMemory m_renderTargetImageMemory{nullptr};
		vk::raii::ImageView    m_renderTargetImageView{nullptr};
		vk::raii::Sampler      m_renderTargetImageSampler{nullptr};

		vk::DescriptorImageInfo m_renderTargetImageDescriptorInfo{nullptr};

		vk::raii::Image        m_renderTargetDepthImage{nullptr};
		vk::raii::DeviceMemory m_renderTargetDepthImageMemory{nullptr};
		vk::raii::ImageView    m_renderTargetDepthImageView{nullptr};

		vk::DescriptorSet m_renderTargetDescriptorSet{nullptr};

		uint32 m_renderTargetImageWidth{0u};
		uint32 m_renderTargetImageHeight{0u};

		// RefPtr<gpu::IFramebuffer> m_framebuffer;

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
