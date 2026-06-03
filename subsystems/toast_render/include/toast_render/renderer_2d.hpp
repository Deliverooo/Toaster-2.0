#pragma once

#include <array>

#include "material.hpp"
#include "render_attachment.hpp"
#include "render_pass.hpp"

#include "toast_gpu/vk/vk_command_buffer.hpp"
#include "toast_gpu/vk/vk_index_buffer.hpp"
#include "toast_gpu/vk/vk_pipeline.hpp"
#include "toast_gpu/vk/vk_vertex_buffer.hpp"

namespace toaster::gpu
{
	class VKLogicalDevice;
	class VKPipeline;
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
		static inline const gpu::VertexBufferLayout quadVbl{
			{gpu::EBufferDataType::eFloat4, "a_Position"},
			{gpu::EBufferDataType::eFloat4, "a_Colour"},
			{gpu::EBufferDataType::eFloat2, "a_TexCoord"},
			{gpu::EBufferDataType::eFloat, "a_TexIndex"},
			{gpu::EBufferDataType::eFloat, "a_TilingFactor"},
		};

		struct Stats
		{
			uint32 quadCount{0u};
		};

		// Ts makes creating renderer 2Ds look better, as, for simple use cases, you only need to specify the viewport size.
		// The rest of the spec info is kept at their default values
		Renderer2D(RenderContext &p_render_ctx, tsm::uint2 p_viewport_size);
		Renderer2D(RenderContext &p_render_ctx, const Renderer2DSpecInfo &p_create_info);
		~Renderer2D();

		auto XM_CALLCONV begin(Dx::FXMMATRIX            p_view, Dx::CXMMATRIX p_projection, RenderingAttachmentInfo *p_override_colour_attachment = nullptr,
							   RenderingAttachmentInfo *p_override_depth_attachment                                                               = nullptr) -> void;
		auto end(gpu::VKCommandBuffer *p_cmd = nullptr) -> void;

		auto XM_CALLCONV submitQuad(Dx::FXMVECTOR p_position, Dx::FXMVECTOR p_scale, const tsm::float4 &p_colour) -> void;
		auto XM_CALLCONV submitQuad(Dx::FXMMATRIX p_transform, const tsm::float4 &p_colour) -> void;
		auto XM_CALLCONV submitQuad(Dx::FXMMATRIX p_transform, const gpu::Texture2DHandle &p_texture, const tsm::float4 &p_colour) -> void;

		auto onResize(tsm::uint2 p_size) -> void;

		auto               getOutputColourTexture() const -> const gpu::Texture2DHandle &;
		[[nodiscard]] auto getStats() const -> const Stats &;

	private:
		auto _construct() -> void; // Called once for each constructor so I don't have to repeat myself

		auto _beginNewBatch() -> void;
		auto _getTextureSlotIndex(const gpu::Texture2DHandle &p_texture) -> uint32;

		NonOwningPtr<RenderContext> m_renderCtx{nullptr};

		Renderer2DSpecInfo m_specInfo{};
		uint32             m_maxVertices;
		uint32             m_maxIndices;

		struct QuadVertex
		{
			Dx::XMFLOAT4 position{0.0f, 0.0f, 0.0f, 0.0f};
			tsm::float4  colour{1.0f};
			tsm::float2  texCoord{0.0f};
			float32      texIndex{0u};
			float32      tilingFactor{1.0f};
		};

		gpu::Texture2DHandle m_renderTargetTexture{nullptr};
		gpu::RawImageHandle  m_renderTargetDepthImage{nullptr};

		gpu::PipelineHandle m_quadPipeline{nullptr};
		RenderPassHandle    m_quadRenderPass{nullptr};
		MaterialHandle      m_quadMaterial{nullptr};

		gpu::VertexBufferHandle m_quadVertexBuffer{nullptr};
		gpu::IndexBufferHandle  m_quadIndexBuffer{nullptr};

		QuadVertex *m_quadVertexBase{nullptr};
		QuadVertex *m_quadVertexPtr{nullptr};

		uint32 m_quadIndexCount{0u};

		std::array<Dx::XMFLOAT4, 4u> m_quadVertexPositions{};
		std::array<tsm::float2, 4u>  m_quadVertexTexCoords{};

		struct CameraUB
		{
			Dx::XMFLOAT4X4 view;
			Dx::XMFLOAT4X4 proj;
		};

		RefPtr<gpu::VKUniformBufferPFF> m_cameraUBs{nullptr};
		std::vector<void *>             m_mappedCameraUBs;

		std::array<gpu::Texture2DHandle, 32u> m_textureSlots;
		uint32                                m_textureSlotIndex{1u};

		RenderingAttachmentInfo *m_colourAttachmentInfo{nullptr};
		RenderingAttachmentInfo *m_depthAttachmentInfo{nullptr};

		Stats m_stats;
	};
}
