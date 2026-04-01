#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "material.hpp"
#include "toast_lib/system_types.h"
#include "toast_gpu/vertex_array.hpp"

namespace toaster
{
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
		uint32 baseVertex;
		uint32 baseIndex;
		uint32 materialIndex;
		uint32 indexCount;
		uint32 vertexCount;

		std::string meshName;
		std::string nodeName;

		glm::mat4 transform{1.0f};
		glm::mat4 localTransform{1.0f};
	};

	struct MeshNode
	{
		uint32              parent = UINT32_MAX;
		std::vector<uint32> children;
		std::vector<uint32> submeshes;

		std::string name;
		glm::mat4   transform{1.0f};
	};

	class Mesh final
	{
	public:
		static RefPtr<Mesh> create(const std::vector<MeshVertex> &p_vertices, const std::vector<uint32> &p_indices);

		Mesh() = default;
		Mesh(const std::vector<MeshVertex> &p_vertices, const std::vector<uint32> &p_indices);

		static RefPtr<Mesh> importFromFile(const io::filesystem::Path &p_path);

		[[nodiscard]] RefPtr<gpu::IVertexBuffer> getVertexBuffer() const;
		[[nodiscard]] RefPtr<gpu::IIndexBuffer>  getIndexBuffer() const;
		[[nodiscard]] RefPtr<gpu::IVertexArray>  getVertexArray() const;

		[[nodiscard]] const std::vector<MeshVertex> &getVertices() const;
		[[nodiscard]] const std::vector<uint32> &    getIndices() const;

		[[nodiscard]] const std::vector<Submesh> &getSubmeshes() const;

		[[nodiscard]] const RefPtr<Material> &getMaterial(uint32 p_index = 0) const;
		std::vector<RefPtr<Material>> getMaterials() const;

	private:
		void traverseNodes(void *p_assimp_node, uint32 p_node_index, const glm::mat4 &p_parent_transform = glm::mat4{1.0f}, uint32 p_level = 0u);

		io::filesystem::Path m_path;

		std::vector<Submesh>  m_submeshes;
		std::vector<MeshNode> m_nodes;

		RefPtr<gpu::IShader>                  m_shader;
		std::vector<RefPtr<gpu::ITexture2D> > m_textures;
		std::vector<RefPtr<Material> >        m_materials;

		std::vector<MeshVertex> m_vertices;
		std::vector<uint32>     m_indices;

		RefPtr<gpu::IVertexBuffer> m_vertexBuffer;
		RefPtr<gpu::IIndexBuffer>  m_indexBuffer;
		RefPtr<gpu::IVertexArray>  m_vertexArray;
	};
}
