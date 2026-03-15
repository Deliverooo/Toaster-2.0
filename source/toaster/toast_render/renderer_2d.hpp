#pragma once

#include "toaster/toast_gpu/framebuffer.hpp"
#include "toaster/toast_gpu/texture.hpp"
#include "toaster/toast_gpu/vertex_array.hpp"

#include "toaster/toast_lib/math/math_vector.hpp"

#include <array>

namespace toaster
{
	struct Renderer2DCreateInfo
	{
		uint32 maxQuads{10000u};

		RefPtr<gpu::Framebuffer> targetFramebuffer{nullptr};
	};

	class Renderer2D
	{
	public:
		Renderer2D(const Renderer2DCreateInfo &p_create_info);
		~Renderer2D();

		void begin(const tsm::float4x4 &p_view_matrix, const tsm::float4x4 &p_proj_matrix);
		void end();

		void submitQuad(const tsm::float3 &p_position, const tsm::float2 &p_scale, const tsm::float4 &p_colour);
		void submitQuad(const tsm::float2 &p_position, const tsm::float2 &p_scale, const tsm::float4 &p_colour);
		void submitQuad(const tsm::float4x4 &p_transform, const tsm::float4 &p_colour);

		void submitQuad(const tsm::float3 &p_position, const tsm::float2 &p_scale, const RefPtr<gpu::Texture2D> &p_texture,
						const tsm::float4 &p_tint_colour = tsm::float4{1.0f, 1.0f, 1.0f, 1.0f});
		void submitQuad(const tsm::float2 &p_position, const tsm::float2 &p_scale, const RefPtr<gpu::Texture2D> &p_texture,
						const tsm::float4 &p_tint_colour = tsm::float4{1.0f, 1.0f, 1.0f, 1.0f});
		void submitQuad(const tsm::float4x4 &p_transform, const RefPtr<gpu::Texture2D> &p_texture,
						const tsm::float4 &  p_tint_colour = tsm::float4{1.0f, 1.0f, 1.0f, 1.0f});

		// TODO: Target framebuffer
		// void setTargetFramebuffer(const RefPtr<gpu::Framebuffer> &p_target_framebuffer);
	private:
		void   _beginNewBatch();
		uint32 _getTextureSlotIndex(const RefPtr<gpu::Texture2D> &p_texture);

		Renderer2DCreateInfo m_createInfo;
		uint32               m_maxVertices;
		uint32               m_maxIndices;

		struct QuadVertex
		{
			tsm::float4 position;
			tsm::float4 colour;
			tsm::float2 texCoord;
			float32     texIndex;
		};

		RefPtr<gpu::VertexArray>  m_quadVertexArray;
		RefPtr<gpu::VertexBuffer> m_quadVertexBuffer;
		RefPtr<gpu::IndexBuffer>  m_quadIndexBuffer;

		QuadVertex *m_quadVertexBase{nullptr};
		QuadVertex *m_quadVertexPtr{nullptr};

		uint32 m_quadIndexCount{0u};

		std::array<tsm::float4, 4u> m_quadVertexPositions;
		std::array<tsm::float2, 4u> m_quadVertexTexCoords;

		static constexpr uint32                               c_maxTextureSlots{32u};
		std::array<RefPtr<gpu::Texture2D>, c_maxTextureSlots> m_textureSlots;
		uint32                                                m_textureSlotIndex{1u};

		RefPtr<gpu::Texture2D> m_whiteTexture;
	};
}
