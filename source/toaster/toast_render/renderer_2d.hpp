#pragma once

#include "toaster/toast_gpu/vertex_array.hpp"
#include "toaster/toast_lib/math/math_vector.hpp"

namespace toaster
{
	class Renderer2D
	{
	public:
		Renderer2D(/*const RefPtr<gpu::Framebuffer> &p_target_framebuffer*/);
		~Renderer2D();

		void begin(const tsm::float4x4 &p_view_matrix, const tsm::float4x4 &p_proj_matrix);
		void end();

		void submitQuad(const tsm::float3 &p_position, const tsm::float2 &p_scale, const tsm::float4 &p_colour);
		void submitQuad(const tsm::float2 &p_position, const tsm::float2 &p_scale, const tsm::float4 &p_colour);

		// TODO: Target framebuffer
		// void setTargetFramebuffer(const RefPtr<gpu::Framebuffer> &p_target_framebuffer);
	private:
		struct QuadVertex
		{
			tsm::float3 position;
			tsm::float2 texCoord;
		};

		RefPtr<gpu::VertexArray>  m_quadVertexArray;
		RefPtr<gpu::VertexBuffer> m_quadVertexBuffer;
		RefPtr<gpu::IndexBuffer>  m_quadIndexBuffer;
	};
}
