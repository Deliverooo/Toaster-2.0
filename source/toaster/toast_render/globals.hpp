#pragma once

#include "shader_library.hpp"

#include "toast_gpu/vk/vk_index_buffer.hpp"
#include "toast_gpu/vk/vk_vertex_buffer.hpp"

#include <glm/glm.hpp>

#include "toast_gpu/vk/vk_texture.hpp"

namespace toaster
{
	class Globals final
	{
	public:
		struct QuadVertex
		{
			glm::vec3 position;
			glm::vec2 texCoord;
		};

		static auto init(gpu::VKGPUContext *p_ctx) -> void;
		static auto shutdown() -> void;

		static auto getShaderLibrary() -> const ShaderLibrary &;

		static auto getFullscreenQuadVertexBuffer() -> const RefPtr<gpu::VKVertexBuffer> &;
		static auto getFullscreenQuadIndexBuffer() -> const RefPtr<gpu::VKIndexBuffer> &;

		static auto getFullscreenQuadVertices() -> const std::vector<QuadVertex> &;
		static auto getFullscreenQuadIndices() -> const std::vector<uint32> &;

		static auto getWhiteTexture() -> const RefPtr<gpu::VKTexture2D> &;
	};
}
