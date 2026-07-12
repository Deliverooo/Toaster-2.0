#pragma once

#include "graphics_state.hpp"
#include "render_attachment.hpp"
#include "storage_buffer.hpp"
#include "uniform_buffer.hpp"

#include "toast_gpu/vk/vk_command_buffer.hpp"

namespace toaster::render
{
	struct TST_RENDER_API DynamicRenderer2DSpecInfo
	{
		tsm::uint2 renderTargetSize{1920u, 1080u};

		uint32 maxQuads{10000u};

		bool msaa{false};
		bool overrideAttachments{false};
	};

	class TST_RENDER_API DynamicRenderer2D
	{
		TST_RENDER_OBJECT
	public:
		static inline const gpu::VertexBufferLayout quadVbl{
			{gpu::EBufferDataType::eFloat4, "a_Position"},
			{gpu::EBufferDataType::eFloat3, "a_Colour"},
			{gpu::EBufferDataType::eFloat2, "a_TexCoord"},
			{gpu::EBufferDataType::eFloat, "a_SamplerIndex"},
			{gpu::EBufferDataType::eFloat, "a_TexIndex"},
			{gpu::EBufferDataType::eFloat, "a_TilingFactor"},
		};

		struct QuadVertex
		{
			Dx::XMFLOAT4 position{0.0f, 0.0f, 0.0f, 1.0f};
			tsm::float3  colour{1.0f};
			tsm::float2  texCoord{0.0f};
			float32      samplerIndex{0u};
			float32      texIndex{0u};
			float32      tilingFactor{1.0f};
		};

		TST_PUSH_CONSTANT_BLOCK(QuadConstants)
		{
			uintptr cameraAddress;
		};

		// Ts makes creating renderer 2Ds look better, as, for simple use cases, you only need to specify the viewport size.
		// The rest of the spec info is kept at their default values
		DynamicRenderer2D(RenderContext &p_render_ctx, tsm::uint2 p_viewport_size);

		DynamicRenderer2D(RenderContext &p_render_ctx, const DynamicRenderer2DSpecInfo &p_create_info);
		~DynamicRenderer2D();

		[[nodiscard]] auto getMSAAColourImage() const -> const gpu::RawImageHandle & { return m_MSAAColourImage; }
		[[nodiscard]] auto getColourImage() const -> const gpu::ImageHandle & { return m_colourImage; }

		[[nodiscard]] auto getMSAADepthImage() const -> const gpu::RawImageHandle & { return m_MSAADepthImage; }
		[[nodiscard]] auto getDepthImage() const -> const gpu::ImageHandle & { return m_depthImage; }

		auto getQuadIndexCount() const -> uint32 { return m_quadIndexCount; }

		auto XM_CALLCONV submitQuad(Dx::FXMMATRIX p_transform, const tsm::float3 &p_colour) -> void;
		auto XM_CALLCONV submitQuad(Dx::FXMVECTOR p_position, Dx::FXMVECTOR p_scale, const tsm::float3 &p_colour) -> void;

		auto XM_CALLCONV submitQuad(Dx::FXMMATRIX      p_transform, const gpu::ImageHandle &p_image, float32 p_tiling_factor = 1.0f,
									const tsm::float3 &p_tint_colour = {1.0f, 1.0f, 1.0f}, gpu::DescriptorSlot p_sampler = UINT32_MAX) -> void;

		auto XM_CALLCONV submitQuad(Dx::FXMVECTOR      p_position, Dx::FXMVECTOR p_scale, const gpu::ImageHandle &p_image, float32 p_tiling_factor = 1.0f,
									const tsm::float3 &p_tint_colour = {1.0f, 1.0f, 1.0f}, gpu::DescriptorSlot p_sampler = UINT32_MAX) -> void;

		auto XM_CALLCONV render(Dx::FXMMATRIX  p_view                    = Dx::XMMatrixIdentity(), Dx::CXMMATRIX p_projection = Dx::XMMatrixIdentity(),
								RenderingInfo *p_override_rendering_info = nullptr) -> void;

		auto onResize(tsm::uint2 p_size) -> void;

	private:
		auto _construct() -> void; // Called once for each constructor, so I don't have to repeat myself

		DynamicRenderer2DSpecInfo m_specInfo{};
		uint32                    m_maxVertices{0u};
		uint32                    m_maxIndices{0u};

		gpu::RawImageHandle m_MSAAColourImage{nullptr};
		gpu::ImageHandle         m_colourImage{nullptr};

		gpu::RawImageHandle m_MSAADepthImage{nullptr};
		gpu::ImageHandle         m_depthImage{nullptr};

		UniformBufferPFFUnique m_cameraUBOs{nullptr};

		GraphicsStateUnique m_graphicsState{nullptr};

		OwningPtr<QuadVertex> m_quadVertexBase{nullptr};
		QuadVertex *          m_quadVertexPtr{nullptr};

		uint32 m_quadIndexCount{0u};

		VertexBufferUnique m_quadVertexBuffer{nullptr};
		gpu::BufferUnique  m_quadIndexBuffer{nullptr};

		std::array<Dx::XMFLOAT4, 4u> m_quadVertexPositions{};
		std::array<tsm::float2, 4u>  m_quadVertexTexCoords{};

		std::array<gpu::ImageHandle, 32u> m_textureSlots;
		uint32                       m_textureSlotIndex{1u};
	};
}
