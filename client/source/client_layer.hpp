#pragma once

#include "editor_camera.hpp"
#include "toaster/toast_kernel/layer.hpp"
#include "toaster/toast_render/renderer_2d.hpp"

#include "toaster/toast_gpu/shader.hpp"
#include "toaster/toast_gpu/texture.hpp"
#include "toaster/toast_kernel/ortho_camera_controller.hpp"

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

		RefPtr<gpu::VKShader>     m_geometryShader{nullptr}; // Shader for geometry, not vk::ShaderStageFlagBits::eGeometry!
		RefPtr<gpu::VKPipeline>   m_geometryPipeline{nullptr};
		RefPtr<gpu::VKRenderPass> m_geometryPass{nullptr};

		gpu::VertexBufferLayout m_quadVertexBufferLayout;
		RefPtr<gpu::VKShader>   m_quadShader{nullptr};
		RefPtr<gpu::VKPipeline> m_quadPipeline{nullptr};

		RefPtr<gpu::VKImage2D> m_colourAttachmentImage{nullptr};
		RefPtr<gpu::VKImage2D> m_depthAttachmentImage{nullptr};

		RefPtr<gpu::VKMesh> m_mesh{nullptr};
		RefPtr<gpu::VKMesh> m_mesh2{nullptr};

		RefPtr<Renderer2D> m_renderer2D{nullptr};

		glm::vec3 m_meshTranslation{0.0f};

		struct CameraUB
		{
			glm::mat4 view;
			glm::mat4 proj;
		};

		RefPtr<gpu::VKUniformBufferPFF> m_ubos;
		std::vector<void *>             m_mappedUniformBuffers;
	};
}
