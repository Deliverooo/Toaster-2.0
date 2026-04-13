#pragma once

#include "toast_gpu/buffer_layout.hpp"
#include "toast_lib/ptr.hpp"
#include "toast_lib/math/math_vector.hpp"

#include <array>
#include <vulkan/vulkan_raii.hpp>

namespace toaster
{
	namespace gpu
	{
		class VKGPUContext;
		class VKPipeline;
		class VKRenderPass;
		class VKMaterial;
		class VKTexture2D;
		class VKImage2D;
		class VKUniformBuffer;
		class VKUniformBufferPFF;
		class VKVertexBuffer;
		class VKIndexBuffer;
		class VKShader;
	}

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

		auto begin(const vk::raii::CommandBuffer &p_cmd, uint32 p_frame_index, const tsm::float4x4 &p_view_matrix, const tsm::float4x4 &p_proj_matrix) -> void;
		auto end(const vk::raii::CommandBuffer &p_cmd, uint32 p_frame_index) -> void;

		auto submitQuad(const tsm::float3 &p_position, const tsm::float2 &p_scale, const tsm::float4 &p_colour) -> void;
		auto submitQuad(const tsm::float2 &p_position, const tsm::float2 &p_scale, const tsm::float4 &p_colour) -> void;
		auto submitQuad(const tsm::float4x4 &p_transform, const tsm::float4 &p_colour) -> void;

		auto onResize(uint32 p_width, uint32 p_height) -> void;

		auto               getColourOutput() const -> const RefPtr<gpu::VKTexture2D> &;
		[[nodiscard]] auto getStats() const -> const Stats &;

	private:
		auto _beginNewBatch() -> void;

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

		gpu::BufferLayout m_quadVertexBufferLayout;

		RefPtr<gpu::VKTexture2D> m_renderTargetTexture{nullptr};
		RefPtr<gpu::VKImage2D>   m_renderTargetDepthImage{nullptr};

		RefPtr<gpu::VKShader>     m_quadShader{nullptr};
		RefPtr<gpu::VKPipeline>   m_quadPipeline{nullptr};
		RefPtr<gpu::VKRenderPass> m_quadRenderPass{nullptr};
		RefPtr<gpu::VKMaterial>   m_quadMaterial{nullptr};

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

		RefPtr<gpu::VKUniformBufferPFF> m_cameraUBs{nullptr};
		std::vector<void *>             m_mappedCameraUBs;

		RefPtr<gpu::VKTexture2D>                  m_whiteTexture{nullptr};
		std::array<RefPtr<gpu::VKTexture2D>, 32u> m_textureSlots;
		uint32                                    m_textureSlotIndex{1u};

		Stats m_stats;
	};
}
