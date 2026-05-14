#pragma once

#include "../toaster_macros.hpp"

#include "shader_library.hpp"

#include "toast_gpu/vk/vk_index_buffer.hpp"
#include "toast_gpu/vk/vk_vertex_buffer.hpp"

#include <glm/glm.hpp>

#include "toast_gpu/vk/vk_texture.hpp"

namespace toaster::render
{
	class TST_API Globals final
	{
		TST_GPU_OBJECT
	public:
		struct QuadVertex
		{
			glm::vec3 position;
			glm::vec2 texCoord;
		};

		Globals(gpu::VKLogicalDevice *p_device, const io::filesystem::Path &p_binary_dir);
		~Globals();

		/*!
		 * @brief use ->get("") to get a shader
		 * The shaders included are "Depth-Pre", "Geometry", "Composite", "Skybox", "Quad", "Compute-Test"
		 * @return Returns the shader library
		 */
		auto shaderLibrary() const -> const ShaderLibrary &;

		auto fullscreenQuadVertexBuffer() const -> const RefPtr<gpu::VKVertexBuffer> &;
		auto fullscreenQuadIndexBuffer() const -> const RefPtr<gpu::VKIndexBuffer> &;

		auto fullscreenQuadVertices() const -> const std::vector<QuadVertex> &;
		auto fullscreenQuadIndices() const -> const std::vector<uint32> &;

		auto whiteTexture() const -> const gpu::VKTexture2D *;
		auto whiteTexture3D() const -> const gpu::VKTexture3D *;

	private:
		io::filesystem::Path m_binaryDir;

		ShaderLibrary m_shaderLibrary;

		RefPtr<gpu::VKVertexBuffer> m_quadVertexBuffer{nullptr};
		RefPtr<gpu::VKIndexBuffer>  m_quadIndexBuffer{nullptr};

		std::vector<QuadVertex> m_quadVertices;
		std::vector<uint32>     m_quadIndices;

		UniquePtr<gpu::VKTexture2D> m_whiteTexture{nullptr};
		UniquePtr<gpu::VKTexture3D> m_whiteTexture3D{nullptr};
	};
}
