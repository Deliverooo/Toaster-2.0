#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "material.hpp"
#include "system_types.h"
#include "io/filesystem.hpp"

#include "vertex_array.hpp"

namespace toaster::gpu
{
	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 textureCoords;
		glm::vec3 tangent;
		glm::vec3 bitangent;
	};

	struct SubMesh
	{
		uint32 indexOffset;
		uint32 indexCount;

		RefPtr<Material> material;

		glm::mat4 transform;
	};

	class Mesh
	{
	public:
		static RefPtr<Mesh> create(const std::vector<Vertex> &p_vertices, const std::vector<uint32> &p_indices);
		static RefPtr<Mesh> create(const io::filesystem::Path &p_path);

		Mesh(const std::vector<Vertex> &p_vertices, const std::vector<uint32> &p_indices);
		Mesh(const io::filesystem::Path &p_path);

		const std::vector<Vertex> & getVertices() const;
		const std::vector<uint32> & getIndices() const;
		const std::vector<SubMesh> &getSubMeshes() const;

		void draw() const;

		const RefPtr<VertexArray> &getVertexArray() const;

	private:
		void calculateTangents();
		void setupMeshData();

		std::vector<SubMesh> m_subMeshes;
		std::vector<Vertex>  m_vertices;
		std::vector<uint32>  m_indices;

		std::vector<RefPtr<Material> > m_materials;

		RefPtr<VertexBuffer> m_vertexBuffer;
		RefPtr<IndexBuffer>  m_indexBuffer;
		RefPtr<VertexArray>  m_vertexArray;

		io::filesystem::Path m_path;
	};
}
