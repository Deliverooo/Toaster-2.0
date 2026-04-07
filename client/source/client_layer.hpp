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
#include "toast_gpu/vk/vk_shader.hpp"
#include "toast_gpu/vk/vk_mesh.hpp"
#include "toast_gpu/vk/vk_pipeline.hpp"
#include "toast_gpu/vk/vk_texture.hpp"
#include "toast_gpu/vk/vk_uniform_buffer.hpp"

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

		void _createDescriptorPool();
		void _createDescriptorSets();

		float32 m_time{0.0f};

		RefPtr<Renderer2D> m_renderer2D{nullptr};

		gpu::VertexBufferLayout m_geometryVertexBufferLayout;
		RefPtr<gpu::VKShader>   m_geometryShader{nullptr}; // Shader for geometry, not vk::ShaderStageFlagBits::eGeometry!
		RefPtr<gpu::VKPipeline> m_geometryPipeline{nullptr};

		gpu::VertexBufferLayout m_quadVertexBufferLayout;
		RefPtr<gpu::VKShader>   m_quadShader{nullptr};
		RefPtr<gpu::VKPipeline> m_quadPipeline{nullptr};

		vk::raii::Image        m_colourAttachmentImage{nullptr};
		vk::raii::DeviceMemory m_colourAttachmentImageMemory{nullptr};
		vk::raii::ImageView    m_colourAttachmentImageView{nullptr};

		RefPtr<gpu::VKTexture2D> m_texture{nullptr};

		struct QuadVertex
		{
			glm::vec3 position;
			glm::vec3 colour;
			glm::vec2 texCoord;
		};

		RefPtr<gpu::VKVertexBuffer> m_quadVertexBuffer{nullptr};
		RefPtr<gpu::VKIndexBuffer>  m_quadIndexBuffer{nullptr};

		RefPtr<gpu::VKMesh> m_mesh{nullptr};

		struct CameraUB
		{
			glm::mat4 model;
			glm::mat4 view;
			glm::mat4 proj;
		};

		RefPtr<gpu::VKUniformBufferPFF> m_ubos;
		std::vector<void *>             m_mappedUniformBuffers;

		struct MaterialCB
		{
			float32 roughness;
		};

		vk::raii::DescriptorPool m_descriptorPool{nullptr};

		std::vector<vk::raii::DescriptorSet> m_descriptorSets;
		std::vector<vk::raii::DescriptorSet> m_compositeDescriptorSets;
	};
}
