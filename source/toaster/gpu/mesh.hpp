#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "system_types.h"
#include "vertex_array.hpp"

namespace toaster::gpu
{
	struct MeshVertex
	{
		glm::vec3 position;
		glm::vec2 texCoord;
	};

	class Mesh final
	{
	public:
		static RefPtr<Mesh> create(const std::vector<MeshVertex> &p_vertices, const std::vector<uint32> &p_indices);
		Mesh(const std::vector<MeshVertex> &p_vertices, const std::vector<uint32> &p_indices);

		[[nodiscard]] RefPtr<VertexBuffer> getVertexBuffer() const;
		[[nodiscard]] RefPtr<IndexBuffer>  getIndexBuffer() const;
		[[nodiscard]] RefPtr<VertexArray>  getVertexArray() const;

		[[nodiscard]] const std::vector<MeshVertex> &getVertices() const;
		[[nodiscard]] const std::vector<uint32> &    getIndices() const;

	private:
		std::vector<MeshVertex> m_vertices;
		std::vector<uint32>     m_indices;

		RefPtr<VertexBuffer> m_vertexBuffer;
		RefPtr<IndexBuffer>  m_indexBuffer;
		RefPtr<VertexArray>  m_vertexArray;
	};
}
