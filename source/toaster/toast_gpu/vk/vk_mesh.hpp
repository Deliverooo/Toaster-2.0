#pragma once

#include <glm/glm.hpp>
#include "vk_index_buffer.hpp"
#include "vk_material.hpp"
#include "vk_texture.hpp"
#include "vk_vertex_buffer.hpp"
#include "toast_lib/io/filesystem.hpp"

namespace toaster::gpu
{
	class VKGPUContext;

	struct MeshVertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 tangent;
		glm::vec3 bitangent;
		glm::vec2 texCoord;
	};

	class VKMesh
	{
	public:
		VKMesh(VKGPUContext *p_ctx, const io::filesystem::Path &p_path, const RefPtr<VKShader> &p_shader); // Shader is temp until I move mesh to the Renderer folder
		VKGPUContext *getContext() const;

		const RefPtr<VKVertexBuffer> &getVertexBuffer() const;
		const RefPtr<VKIndexBuffer> & getIndexBuffer() const;

		const RefPtr<VKMaterial> &getMaterial() const;
		// const RefPtr<VKTexture2D> &getAlbedoMap() const;

		// float32 getRoughness() const;

		const std::vector<MeshVertex> &getVertices() const;
		const std::vector<uint16> &    getIndices() const;

	private:
		VKGPUContext *m_ctx{nullptr};

		io::filesystem::Path m_path;

		std::vector<MeshVertex> m_vertices;
		std::vector<uint16>     m_indices;

		RefPtr<VKVertexBuffer> m_vertexBuffer{nullptr};
		RefPtr<VKIndexBuffer>  m_indexBuffer{nullptr};

		RefPtr<VKMaterial> m_material{nullptr};

		RefPtr<VKTexture2D> m_albedoMap{nullptr};
		// float32             m_roughness{0.0f};
	};
}
