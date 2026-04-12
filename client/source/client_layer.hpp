#pragma once

#include "editor_camera.hpp"
#include "toaster/toast_kernel/layer.hpp"
#include "toaster/toast_render/renderer_2d.hpp"

#include "toaster/toast_gpu/shader.hpp"
#include "toaster/toast_gpu/texture.hpp"

#include "toaster/toast_lib/events/key_event.hpp"
#include "toaster/toast_lib/events/mouse_event.hpp"
#include "toaster/toast_lib/events/window_event.hpp"

#include "toast_gpu/vk/vk_gpu_context.hpp"
#include "toast_gpu/vk/vk_vertex_buffer.hpp"
#include "toast_gpu/vk/vk_index_buffer.hpp"
#include "toast_gpu/vk/vk_material.hpp"
#include "toast_gpu/vk/vk_shader.hpp"
#include "toast_gpu/vk/vk_mesh.hpp"
#include "toast_gpu/vk/vk_pipeline.hpp"
#include "toast_gpu/vk/vk_render_pass.hpp"
#include "toast_gpu/vk/vk_texture.hpp"
#include "toast_gpu/vk/vk_uniform_buffer.hpp"
#include "toast_gpu/vk/vk_render_attachment.hpp"
#include "toast_scene/scene_renderer.hpp"

namespace toaster
{
	class ClientLayer final : public IAppLayer
	{
	public:
		ClientLayer(Application *p_app);

		void onInit() override;
		void onDestroy() override;
		void onUpdate(float32 p_dt) override;
		void onEvent(Event &p_event) override;

		void onUIRender() override;

	private:
		bool onKeyPressEvent(KeyPressEvent &e);
		bool onWindowResizeEvent(WindowResizeEvent &e);

		float32 m_time{0.0f};

		uint32 m_viewportWidth{0u};
		uint32 m_viewportHeight{0u};

		#pragma region fullscreen pass
		RefPtr<gpu::VKPipeline>   m_compositePipeline{nullptr};
		RefPtr<gpu::VKRenderPass> m_fullscreenPass{nullptr};

		RefPtr<gpu::VKMaterial> m_fullscreenMaterial{nullptr};

		RefPtr<gpu::VKImage2D> m_MSAAColourAttachmentImage{nullptr};
		RefPtr<gpu::VKImage2D> m_MSAADepthAttachmentImage{nullptr};
		#pragma  endregion


		RefPtr<gpu::VKMesh> m_mesh{nullptr};
		RefPtr<gpu::VKMesh> m_mesh2{nullptr};

		RefPtr<Renderer2D> m_renderer2D{nullptr};

		glm::vec3 m_meshTranslation{0.0f};

		struct CameraUB
		{
			glm::mat4 view;
			glm::mat4 proj;
		};
		RefPtr<SceneRenderer> m_sceneRenderer{nullptr};

		EditorCamera m_editorCamera;
	};
}
