#pragma once

#include "toast_gpu/framebuffer.hpp"
#include "toast_gpu/texture.hpp"
#include "toast_gpu/vertex_array.hpp"
#include "toast_gpu/vk/vk_gpu_context.hpp"

#include "toast_lib/camera.hpp"
#include "toast_lib/math/math_vector.hpp"

#include <array>

#include "toast_gpu/vk/vk_image.hpp"
#include "toast_gpu/vk/vk_pipeline.hpp"
#include "toast_gpu/vk/vk_vertex_buffer.hpp"
#include "toast_gpu/vk/vk_index_buffer.hpp"
#include "toast_gpu/vk/vk_texture.hpp"
#include "toast_gpu/vk/vk_uniform_buffer.hpp"

namespace toaster
{
	struct Renderer2DCreateInfo
	{
		uint32 maxQuads{10000u};

		uint32 renderTargetWidth{1920u};
		uint32 renderTargetHeight{1080u};
	};

	class Renderer2D final
	{
	public:
		struct Stats
		{
			uint32 quadCount{0u};
		};

		explicit Renderer2D(gpu::VKGPUContext *p_ctx, const Renderer2DCreateInfo &p_create_info);
		~Renderer2D();

		void begin(vk::raii::CommandBuffer &p_cmd, uint32 p_frame_index, const tsm::float4x4 &p_view_matrix, const tsm::float4x4 &p_proj_matrix);
		void end(vk::raii::CommandBuffer &p_cmd, uint32 p_frame_index);

		void submitQuad(const tsm::float3 &p_position, const tsm::float2 &p_scale, const tsm::float4 &p_colour);
		void submitQuad(const tsm::float2 &p_position, const tsm::float2 &p_scale, const tsm::float4 &p_colour);
		void submitQuad(const tsm::float4x4 &p_transform, const tsm::float4 &p_colour);

		[[nodiscard]] const Stats &getStats() const;

		vk::raii::Image &        getRenderTargetImage();
		vk::raii::DeviceMemory & getRenderTargetImageMemory();
		vk::raii::ImageView &    getRenderTargetImageView();
		vk::DescriptorImageInfo &getRenderTargetDescriptorImageInfo();

		void onResize(uint32 p_width, uint32 p_height);

	private:
		void _beginNewBatch();
		void _createRenderTargetResources();

		void _createDescriptorPool();
		void _createDescriptorSets();

		gpu::VKGPUContext *m_ctx;

		Renderer2DCreateInfo m_createInfo;
		uint32               m_maxVertices;
		uint32               m_maxIndices;

		struct QuadVertex
		{
			tsm::float4 position;
			tsm::float4 colour;
			tsm::float2 texCoord;
			float32     texIndex;
			float32     tilingFactor;
		};

		gpu::VertexBufferLayout m_quadVertexBufferLayout;

		vk::raii::Image         m_renderTargetImage{nullptr};
		vk::raii::DeviceMemory  m_renderTargetImageMemory{nullptr};
		vk::raii::ImageView     m_renderTargetImageView{nullptr};
		vk::raii::Sampler       m_renderTargetImageSampler{nullptr};
		vk::DescriptorImageInfo m_renderTargetDescriptorImageInfo{nullptr};

		vk::raii::Image        m_renderTargetDepthImage{nullptr};
		vk::raii::DeviceMemory m_renderTargetDepthImageMemory{nullptr};
		vk::raii::ImageView    m_renderTargetDepthImageView{nullptr};

		RefPtr<gpu::VKShader>   m_quadShader{nullptr};
		RefPtr<gpu::VKPipeline> m_quadPipeline{nullptr};

		RefPtr<gpu::VKVertexBuffer> m_quadVertexBuffer{nullptr};
		RefPtr<gpu::VKIndexBuffer>  m_quadIndexBuffer{nullptr};

		QuadVertex *m_quadVertexBase{nullptr};
		QuadVertex *m_quadVertexPtr{nullptr};

		uint32 m_quadIndexCount{0u};

		std::array<tsm::float4, 4u> m_quadVertexPositions;
		std::array<tsm::float2, 4u> m_quadVertexTexCoords;

		struct CameraUB
		{
			glm::mat4 view;
			glm::mat4 proj;
		};

		RefPtr<gpu::VKUniformBufferPFF> m_uniformBuffers{nullptr};
		std::vector<void *>             m_mappedUniformBuffers;

		vk::raii::DescriptorPool             m_descriptorPool{nullptr};
		std::vector<vk::raii::DescriptorSet> m_descriptorSets;

		RefPtr<gpu::VKTexture2D>                  m_whiteImage{nullptr};
		std::array<RefPtr<gpu::VKTexture2D>, 32u> m_textureSlots;
		uint32                                  m_textureSlotIndex{1u};

		Stats m_stats;
	};
}
