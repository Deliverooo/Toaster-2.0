#pragma once

#include "toaster/toast_kernel/layer.hpp"
#include "toaster/toast_render/renderer_2d.hpp"

#include "toaster/toast_gpu/shader.hpp"
#include "toaster/toast_gpu/texture.hpp"
#include "toaster/toast_kernel/ortho_camera_controller.hpp"

#include "toaster/toast_lib/events/key_event.hpp"
#include "toaster/toast_lib/events/mouse_event.hpp"
#include "toaster/toast_lib/events/window_event.hpp"

#include "toast_gpu/vk/vk_gpu_context.hpp"

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

	private:
		bool onKeyPressEvent(KeyPressEvent &e);

		void _recordCommandBuffer(uint32 p_image_index);

		static vk::Format _getVulkanAttribType(gpu::EShaderDataType p_type);

		void _createDescriptorSetLayout();
		void _createGraphicsPipeline();
		void _createVertexBuffer();
		void _createIndexBuffer();
		void _createUniformBuffers();
		void _createDescriptorPool();
		void _createDescriptorSets();

		float32 m_time{0.0f};

		gpu::VertexBufferLayout m_vertexBufferLayout;

		vk::raii::DescriptorSetLayout m_descriptorSetLayout{nullptr};

		vk::raii::Pipeline       m_graphicsPipeline{nullptr};
		vk::raii::PipelineLayout m_pipelineLayout{nullptr};

		struct Vertex
		{
			glm::vec3 position;
			glm::vec3 colour;
		};

		const std::vector<Vertex> m_vertices{
			{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 0.0f}},
			{{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 1.0f}},
			{{-0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 1.0f}}
		};

		const std::vector<uint16> m_indices{0, 1, 2, 2, 3, 0};

		vk::raii::Buffer       m_vertexBuffer{nullptr};
		vk::raii::DeviceMemory m_vertexBufferMemory{nullptr};

		vk::raii::Buffer       m_indexBuffer{nullptr};
		vk::raii::DeviceMemory m_indexBufferMemory{nullptr};

		struct UniformBufferObject
		{
			glm::mat4 model;
			glm::mat4 view;
			glm::mat4 proj;
		};

		std::vector<vk::raii::Buffer>       m_uniformBuffers;
		std::vector<vk::raii::DeviceMemory> m_uniformBufferMemories;
		std::vector<void *>                 m_mappedUniformBuffers;

		struct FrameDataUB
		{
			glm::vec2 resolution;
			float32   time;
		};

		vk::raii::DescriptorPool             m_descriptorPool{nullptr};
		std::vector<vk::raii::DescriptorSet> m_descriptorSets;
	};
}
