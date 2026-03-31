#pragma once

#include "toast_gpu/framebuffer.hpp"
#include "toast_gpu/texture.hpp"
#include "toast_gpu/vertex_array.hpp"

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

		#ifndef TST_RENDERER_2D_USE_64_BIT_IDS
		using IDType = int32;
		static constexpr IDType c_invalidID{-1};
		#else
		using IDType = int64; static constexpr IDType c_invalidID{-1};
		#endif

		explicit Renderer2D(const Renderer2DCreateInfo &p_create_info);
		~Renderer2D();

		void begin(const Camera &p_camera, const tsm::float4x4 &p_transform);
		void begin(const tsm::float4x4 &p_view_matrix, const tsm::float4x4 &p_proj_matrix);
		void end();

		void submitQuad(const tsm::float3 &p_position, const tsm::float2 &p_scale, const tsm::float4 &p_colour, IDType p_object_id = c_invalidID);
		void submitQuad(const tsm::float2 &p_position, const tsm::float2 &p_scale, const tsm::float4 &p_colour, IDType p_object_id = c_invalidID);
		void submitQuad(const tsm::float4x4 &p_transform, const tsm::float4 &p_colour, IDType p_object_id = c_invalidID);

		void submitQuad(const tsm::float3 &p_position, const tsm::float2 &p_scale, const RefPtr<gpu::ITexture2D> &p_texture,
						const tsm::float4 &p_tint_colour = tsm::float4{1.0f, 1.0f, 1.0f, 1.0f}, float32 p_tiling_factor = 1.0f, IDType p_object_id = c_invalidID);
		void submitQuad(const tsm::float2 &p_position, const tsm::float2 &                              p_scale, const RefPtr<gpu::ITexture2D> &p_texture,
						const tsm::float4 &p_tint_colour = tsm::float4{1.0f, 1.0f, 1.0f, 1.0f}, float32 p_tiling_factor = 1.0f, IDType p_object_id = c_invalidID);
		void submitQuad(const tsm::float4x4 &p_transform, const RefPtr<gpu::ITexture2D> &                 p_texture,
						const tsm::float4 &  p_tint_colour = tsm::float4{1.0f, 1.0f, 1.0f, 1.0f}, float32 p_tiling_factor = 1.0f, IDType p_object_id = c_invalidID);

		const Stats &getStats() const;

	private:
		void   _beginNewBatch();
		uint32 _getTextureSlotIndex(const RefPtr<gpu::ITexture2D> &p_texture);

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
			IDType      objectID;
		};

		RefPtr<gpu::IVertexArray>  m_quadVertexArray;
		RefPtr<gpu::IVertexBuffer> m_quadVertexBuffer;
		RefPtr<gpu::IIndexBuffer>  m_quadIndexBuffer;

		QuadVertex *m_quadVertexBase{nullptr};
		QuadVertex *m_quadVertexPtr{nullptr};

		uint32 m_quadIndexCount{0u};

		std::array<tsm::float4, 4u> m_quadVertexPositions;
		std::array<tsm::float2, 4u> m_quadVertexTexCoords;

		static constexpr uint32                                c_maxTextureSlots{32u};
		std::array<RefPtr<gpu::ITexture2D>, c_maxTextureSlots> m_textureSlots;
		uint32                                                 m_textureSlotIndex{1u};

		RefPtr<gpu::ITexture2D> m_whiteTexture;

		Stats m_stats;
	};
}
