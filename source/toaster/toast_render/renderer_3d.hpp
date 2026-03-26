#pragma once

#include "toast_gpu/texture.hpp"
#include "toast_gpu/vertex_array.hpp"

#include "toast_lib/math/math_vector.hpp"

#include <array>

namespace toaster
{
	struct Renderer3DCreateInfo
	{
		uint32 maxCubes{10000u};
	};

	class Renderer3D
	{
	public:
		Renderer3D(const Renderer3DCreateInfo &p_create_info);
		~Renderer3D();

		void begin(const tsm::float4x4 &p_view_matrix, const tsm::float4x4 &p_proj_matrix);
		void end();


		void submitCube(const tsm::float3 &p_position, const tsm::float2 &p_scale, const tsm::float4 &p_colour);
		void submitCube(const tsm::float2 &p_position, const tsm::float2 &p_scale, const tsm::float4 &p_colour);

		void submitCube(const tsm::float3 &p_position, const tsm::float2 &p_scale, const RefPtr<gpu::Texture2D> &p_texture,
						const tsm::float4 &p_tint_colour = tsm::float4{1.0f, 1.0f, 1.0f, 1.0f});
		void submitCube(const tsm::float2 &p_position, const tsm::float2 &p_scale, const RefPtr<gpu::Texture2D> &p_texture,
						const tsm::float4 &p_tint_colour = tsm::float4{1.0f, 1.0f, 1.0f, 1.0f});

		// TODO: Target framebuffer
		// void setTargetFramebuffer(const RefPtr<gpu::Framebuffer> &p_target_framebuffer);
	private:
		void _beginNewBatch();
		uint32 _getTextureSlotIndex(const RefPtr<gpu::Texture2D> &p_texture);

		Renderer3DCreateInfo m_createInfo;
		uint32               m_maxVertices;
		uint32               m_maxIndices;

		struct CubeVertex
		{
			tsm::float4 position;
			tsm::float4 colour;
			tsm::float2 texCoord;
			float32     texIndex;
		};

		RefPtr<gpu::VertexArray>  m_cubeVertexArray;
		RefPtr<gpu::VertexBuffer> m_cubeVertexBuffer;
		RefPtr<gpu::IndexBuffer>  m_cubeIndexBuffer;


		CubeVertex *m_cubeVertexBase{nullptr};
		CubeVertex *m_cubeVertexPtr{nullptr};

		uint32 m_cubeIndexCount{0u};

		std::array<tsm::float4, 4u> m_cubeVertexPositions;
		std::array<tsm::float2, 4u> m_cubeVertexTexCoords;

		static constexpr uint32                               c_maxTextureSlots{32u};
		std::array<RefPtr<gpu::Texture2D>, c_maxTextureSlots> m_textureSlots;
		uint32                                                m_textureSlotIndex{1u};

		RefPtr<gpu::Texture2D> m_whiteTexture;
	};
}
