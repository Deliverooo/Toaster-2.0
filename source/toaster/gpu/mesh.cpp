#include "mesh.hpp"

namespace toaster::gpu
{
	RefPtr<Mesh> Mesh::create(const std::vector<MeshVertex> &p_vertices, const std::vector<uint32> &p_indices)
	{
		return make_reference<Mesh>(p_vertices, p_indices);
	}

	Mesh::Mesh(const std::vector<MeshVertex> &p_vertices, const std::vector<uint32> &p_indices) : m_vertices(p_vertices), m_indices(p_indices)
	{
		m_vertexBuffer = VertexBuffer::create(m_vertices.data(), m_vertices.size() * sizeof(MeshVertex));
		const auto vbl = VertexBufferLayout{{EShaderDataType::eFloat3, "a_Position"}, {EShaderDataType::eFloat2, "a_TexCoord"}};
		m_vertexBuffer->setLayout(vbl);

		m_indexBuffer = IndexBuffer::create(m_indices.data(), m_indices.size());

		m_vertexArray = VertexArray::create();
		m_vertexArray->addVertexBuffer(m_vertexBuffer);
		m_vertexArray->setIndexBuffer(m_indexBuffer);
	}

	RefPtr<VertexBuffer> Mesh::getVertexBuffer() const
	{
		return m_vertexBuffer;
	}

	RefPtr<IndexBuffer> Mesh::getIndexBuffer() const
	{
		return m_indexBuffer;
	}

	RefPtr<VertexArray> Mesh::getVertexArray() const
	{
		return m_vertexArray;
	}

	const std::vector<MeshVertex> &Mesh::getVertices() const
	{
		return m_vertices;
	}

	const std::vector<uint32> &Mesh::getIndices() const
	{
		return m_indices;
	}
}
