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

		static void init(gpu::VKGPUContext *p_ctx);
		static void shutdown();

		static const ShaderLibrary &getShaderLibrary();

		static const RefPtr<gpu::VKVertexBuffer> &getFullscreenQuadVertexBuffer();
		static const RefPtr<gpu::VKIndexBuffer> & getFullscreenQuadIndexBuffer();

		static const std::vector<QuadVertex> &getFullscreenQuadVertices();
		static const std::vector<uint16> &    getFullscreenQuadIndices();

		static const RefPtr<gpu::VKTexture2D>& getWhiteTexture();
	};
}
