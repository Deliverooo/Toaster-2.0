#include "mesh.hpp"

#include <openglhpp/opengl.hpp>

#include "globals.hpp"
#include "logging.hpp"

namespace toaster::gpu
{
	RefPtr<Mesh> Mesh::create(const std::vector<Vertex> &p_vertices, const std::vector<uint32> &p_indices)
	{
		return std::make_shared<Mesh>(p_vertices, p_indices);
	}

	RefPtr<Mesh> Mesh::create(const io::filesystem::Path &p_path)
	{
		return std::make_shared<Mesh>(p_path);
	}

	Mesh::Mesh(const std::vector<Vertex> &p_vertices, const std::vector<uint32> &p_indices) : m_vertices(p_vertices), m_indices(p_indices)
	{
		SubMesh &sub_mesh    = m_subMeshes.emplace_back();
		sub_mesh.indexOffset = 0u;
		sub_mesh.indexCount  = p_indices.size();
		sub_mesh.material    = Globals::defaultMaterial();

		m_materials.push_back(Globals::defaultMaterial());

		calculateTangents();
		setupMeshData();
	}

	Mesh::Mesh(const io::filesystem::Path &p_path)
	{
	}

	const std::vector<Vertex> &Mesh::getVertices() const
	{
		return m_vertices;
	}

	const std::vector<uint32> &Mesh::getIndices() const
	{
		return m_indices;
	}

	const std::vector<SubMesh> &Mesh::getSubMeshes() const
	{
		return m_subMeshes;
	}

	void Mesh::draw() const
	{
		m_vertexArray->bind();
		gl::drawElements(gl::DrawMode::eTriangles, static_cast<gl::SizeI>(m_indexBuffer->getIndexCount()), gl::DataType::eUnsignedInt, nullptr);
	}

	const RefPtr<VertexArray> &Mesh::getVertexArray() const
	{
		return m_vertexArray;
	}

	void Mesh::calculateTangents()
	{
		// Early return if no indices or vertices
		if (m_indices.empty() || m_vertices.empty())
		{
			LOG_WARN("Cannot calculate tangents: empty indices or vertices");
			return;
		}

		// Calculate tangents and bitangents for normal mapping
		for (size_t i = 0; i < m_indices.size(); i += 3)
		{
			if (i + 2 >= m_indices.size())
				break;

			// Validate indices are within bounds
			uint32_t idx0 = m_indices[i + 0];
			uint32_t idx1 = m_indices[i + 1];
			uint32_t idx2 = m_indices[i + 2];

			if (idx0 >= m_vertices.size() || idx1 >= m_vertices.size() || idx2 >= m_vertices.size())
			{
				LOG_ERROR("Vertex index out of range in face {}: indices [{}, {}, {}], vertex count: {}", i / 3, idx0, idx1, idx2, m_vertices.size());
				continue; // Skip this triangle
			}

			Vertex &v0 = m_vertices[idx0];
			Vertex &v1 = m_vertices[idx1];
			Vertex &v2 = m_vertices[idx2];

			glm::vec3 edge1 = v1.position - v0.position;
			glm::vec3 edge2 = v2.position - v0.position;

			glm::vec2 deltaUV1 = v1.textureCoords - v0.textureCoords;
			glm::vec2 deltaUV2 = v2.textureCoords - v0.textureCoords;

			float denominator = (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
			float f           = (denominator != 0.0f) ? 1.0f / denominator : 0.0f;

			glm::vec3 tangent;
			tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
			tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
			tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

			glm::vec3 bitangent;
			bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
			bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
			bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

			v0.tangent = tangent;
			v1.tangent = tangent;
			v2.tangent = tangent;

			v0.bitangent = bitangent;
			v1.bitangent = bitangent;
			v2.bitangent = bitangent;
		}
	}

	void Mesh::setupMeshData()
	{
		m_vertexArray = VertexArray::create();

		m_vertexBuffer = VertexBuffer::create(m_vertices.size() * sizeof(Vertex));
		m_vertexBuffer->setData(m_vertices.data(), m_vertices.size() * sizeof(Vertex));

		m_vertexBuffer->setLayout({
									  {EShaderDataType::eFloat3, "a_Position"},
									  {EShaderDataType::eFloat3, "a_Normal"},
									  {EShaderDataType::eFloat2, "a_TexCoord"},
									  {EShaderDataType::eFloat3, "a_Tangent"},
									  {EShaderDataType::eFloat3, "a_Bitangent"}
								  });

		m_vertexArray->addVertexBuffer(m_vertexBuffer);

		m_indexBuffer = IndexBuffer::create(m_indices.data(), static_cast<uint32_t>(m_indices.size()));
		m_vertexArray->setIndexBuffer(m_indexBuffer);
	}
}
