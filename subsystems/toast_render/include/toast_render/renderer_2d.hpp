#pragma once

#include "material.hpp"

#include <array>

namespace toaster::gpu
{
	class VKLogicalDevice;
	class VKPipeline;
	class VKRenderPass;
	class VKMaterial;
	class VKTexture2D;
	class VKRawImage;
	class VKUniformBuffer;
	class VKUniformBufferPFF;
	class VKVertexBuffer;
	class VKIndexBuffer;
	class VKShader;
}

namespace toaster::render
{
	class RenderContext;

	struct TST_RENDER_API Renderer2DSpecInfo
	{
		tsm::uint2 renderTargetSize{1920u, 1080u};

		uint32 maxQuads{10000u};

		bool msaa{false};
		bool overrideAttachments{false};
	};

	class TST_RENDER_API Renderer2D final
	{
	public:
		struct Stats
		{
			uint32 quadCount{0u};
		};

		explicit Renderer2D(RenderContext *p_render_ctx, const Renderer2DSpecInfo &p_create_info);
		~Renderer2D();

		auto begin(const tsm::float4x4 &p_view_matrix, const tsm::float4x4 &p_proj_matrix) -> void;
		auto end(gpu::VKCommandBuffer *              p_cmd, gpu::RenderingAttachmentInfo *p_override_colour_attachment = nullptr,
				 const gpu::RenderingAttachmentInfo *p_override_depth_attachment                                       = nullptr) -> void;

		auto submitQuad(const tsm::float3 &p_position, const tsm::float2 &p_scale, const tsm::float4 &p_colour) -> void;
		auto submitQuad(const tsm::float2 &p_position, const tsm::float2 &p_scale, const tsm::float4 &p_colour) -> void;
		auto submitQuad(const tsm::float4x4 &p_transform, const tsm::float4 &p_colour) -> void;
		auto submitQuad(const tsm::float4x4 &p_transform, const gpu::Texture2DHandle &p_texture, const tsm::float4 &p_colour) -> void;

		auto onResize(tsm::uint2 p_size) -> void;

		auto               getOutputColourTexture() const -> const gpu::Texture2DHandle &;
		[[nodiscard]] auto getStats() const -> const Stats &;

	private:
		auto _beginNewBatch() -> void;
		auto _getTextureSlotIndex(const gpu::Texture2DHandle &p_texture) -> uint32;

		NonOwningPtr<RenderContext> m_renderCtx{nullptr};

		Renderer2DSpecInfo m_specInfo{};
		uint32             m_maxVertices;
		uint32             m_maxIndices;

		struct QuadVertex
		{
			tsm::float4 position{0.0f};
			tsm::float4 colour{1.0f};
			tsm::float2 texCoord{0.0f};
			float32     texIndex{0u};
			float32     tilingFactor{1.0f};
		};

		gpu::BufferLayout m_quadVertexBufferLayout;

		gpu::Texture2DHandle m_renderTargetTexture{nullptr};
		gpu::RawImageHandle  m_renderTargetDepthImage{nullptr};

		gpu::PipelineHandle   m_quadPipeline{nullptr};
		gpu::RenderPassHandle m_quadRenderPass{nullptr};
		MaterialHandle        m_quadMaterial{nullptr};

		gpu::VertexBufferHandle m_quadVertexBuffer{nullptr};
		gpu::IndexBufferHandle  m_quadIndexBuffer{nullptr};

		QuadVertex *m_quadVertexBase{nullptr};
		QuadVertex *m_quadVertexPtr{nullptr};

		uint32 m_quadIndexCount{0u};

		std::array<tsm::float4, 4u> m_quadVertexPositions{};
		std::array<tsm::float2, 4u> m_quadVertexTexCoords{};

		struct CameraUB
		{
			tsm::float4x4 view{1.0f};
			tsm::float4x4 proj{1.0f};
		};

		RefPtr<gpu::VKUniformBufferPFF> m_cameraUBs{nullptr};
		std::vector<void *>             m_mappedCameraUBs;

		std::array<gpu::Texture2DHandle, 32u> m_textureSlots;
		uint32                                m_textureSlotIndex{1u};

		Stats m_stats;
	};
}
