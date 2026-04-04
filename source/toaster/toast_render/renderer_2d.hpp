#pragma once

#include "toast_gpu/framebuffer.hpp"
#include "toast_gpu/texture.hpp"
#include "toast_gpu/vertex_array.hpp"
#include "toast_gpu/vk/vk_gpu_context.hpp"

#include "toast_lib/camera.hpp"
#include "toast_lib/math/math_vector.hpp"

#include <array>

namespace toaster
{
	struct Renderer2DCreateInfo
	{
		uint32 maxQuads{10000u};
	};

	class Renderer2D
	{
	public:
		struct Stats
		{
			uint32 quadCount{0u};
		};

		explicit Renderer2D(gpu::VKGPUContext *p_ctx, const Renderer2DCreateInfo &p_create_info);
		~Renderer2D();

		void begin(vk::raii::CommandBuffer& p_cmd, const tsm::float4x4 &p_view_matrix, const tsm::float4x4 &p_proj_matrix);
		void end(vk::raii::CommandBuffer& p_cmd);

		void submitQuad(const tsm::float3 &p_position, const tsm::float2 &p_scale, const tsm::float4 &p_colour);
		void submitQuad(const tsm::float2 &p_position, const tsm::float2 &p_scale, const tsm::float4 &p_colour);
		void submitQuad(const tsm::float4x4 &p_transform, const tsm::float4 &p_colour);

		[[nodiscard]] const Stats &getStats() const;

		vk::raii::Image &       getRenderTargetImage();
		vk::raii::DeviceMemory &getRenderTargetImageMemory();
		vk::raii::ImageView &   getRenderTargetImageView();

	private:
		void _beginNewBatch();

		gpu::VKGPUContext *m_ctx;

		Renderer2DCreateInfo m_createInfo;
		uint32               m_maxVertices;
		uint32               m_maxIndices;

		struct QuadVertex
		{
			tsm::float4 position;
			tsm::float4 colour;
			tsm::float2 texCoord;
		};

		gpu::VertexBufferLayout m_quadVertexBufferLayout;

		vk::raii::DescriptorSetLayout m_quadDescriptorSetLayout{nullptr};

		vk::raii::Image        m_renderTargetImage{nullptr};
		vk::raii::DeviceMemory m_renderTargetImageMemory{nullptr};
		vk::raii::ImageView    m_renderTargetImageView{nullptr};

		vk::raii::Pipeline       m_quadPipeline{nullptr};
		vk::raii::PipelineLayout m_quadPipelineLayout{nullptr};

		vk::raii::Buffer       m_quadVertexBuffer{nullptr};
		vk::raii::DeviceMemory m_quadVertexBufferMemory{nullptr};
		void *                 m_mappedQuadVertexBufferMemory{nullptr};

		vk::raii::Buffer       m_quadIndexBuffer{nullptr};
		vk::raii::DeviceMemory m_quadIndexBufferMemory{nullptr};

		QuadVertex *m_quadVertexBase{nullptr};
		QuadVertex *m_quadVertexPtr{nullptr};

		struct CameraUB
		{
			glm::mat4 model;
			glm::mat4 view;
			glm::mat4 proj;
		};

		std::vector<vk::raii::Buffer>       m_uniformBuffers;
		std::vector<vk::raii::DeviceMemory> m_uniformBufferMemories;
		std::vector<void *>                 m_mappedUniformBuffers;

		uint32 m_quadIndexCount{0u};

		std::array<tsm::float4, 4u> m_quadVertexPositions;
		std::array<tsm::float2, 4u> m_quadVertexTexCoords;

		Stats m_stats;
	};
}
