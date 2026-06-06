#pragma once

#include "toast_render.hpp"

#include "toast_gpu/buffer_layout.hpp"
#include "toast_gpu/vk/vk_command_buffer.hpp"

#include "toast_gpu/vk/vk_shader.hpp"

namespace toaster::render
{
	class RenderContext;

	class TST_RENDER_API GraphicsState
	{
	public:
		GraphicsState(RenderContext &p_render_ctx,  std::vector<gpu::DynamicShaderHandle> p_shaders);

		auto setVertexBufferLayout(const gpu::VertexBufferLayout &p_layout) -> GraphicsState &
		{
			m_vertexBufferLayout = p_layout;
			return *this;
		}

		auto bind(gpu::VKCommandBuffer *p_command_buffer = nullptr) const -> void;

	private:
		NonOwningPtr<RenderContext> m_renderCtx{nullptr};

		std::vector<gpu::DynamicShaderHandle> m_shaders;

		gpu::VertexBufferLayout m_vertexBufferLayout;
	};
}
