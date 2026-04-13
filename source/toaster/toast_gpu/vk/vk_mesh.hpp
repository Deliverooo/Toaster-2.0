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

	struct Submesh
	{
		String name{};

		glm::mat4 transform{1.0f};
		glm::mat4 localTransform{1.0f};

		uint32 baseVertex{0u};
		uint32 baseIndex{0u};
		uint32 materialIndex{0u};
		uint32 indexCount{0u};
		uint32 vertexCount{0u};
	};

	struct MeshNode
	{
		String    name{};
		glm::mat4 localTransform{1.0f};

		std::vector<uint32> children;
		std::vector<uint32> submeshes;
		uint32              parent{UINT32_MAX};
	};

	class VKMesh
	{
	public:
		VKMesh(VKGPUContext *p_ctx, const io::filesystem::Path &p_path, const RefPtr<VKShader> &p_shader); // Shader is temp until I move mesh to the Renderer folder
		auto getContext() const -> VKGPUContext *;

		auto getVertexBuffer() const -> const RefPtr<VKVertexBuffer> &;
		auto getIndexBuffer() const -> const RefPtr<VKIndexBuffer> &;

		auto getMaterials() const -> const std::vector<RefPtr<VKMaterial> > &;
		auto getSubmeshes() const -> const std::vector<Submesh> &;

		auto getVertices() const -> const std::vector<MeshVertex> &;
		auto getIndices() const -> const std::vector<uint16> &;

	private:
		auto _traverseNodes(void *p_assimp_node, uint32 p_node_index, const glm::mat4 &p_parent_transform, uint32 p_level) -> void;

		VKGPUContext *m_ctx{nullptr};

		io::filesystem::Path m_path;

		std::vector<Submesh>  m_submeshes;
		std::vector<MeshNode> m_nodes;

		std::vector<MeshVertex> m_vertices;
		std::vector<uint16>     m_indices;

		RefPtr<VKVertexBuffer> m_vertexBuffer{nullptr};
		RefPtr<VKIndexBuffer>  m_indexBuffer{nullptr};

		std::vector<RefPtr<VKMaterial> > m_materials;
	};
}
