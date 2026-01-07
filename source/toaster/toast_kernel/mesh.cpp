#include "mesh.hpp"
#include "gpu_context.hpp"
#include "logging.hpp"

#include <cstring>
#include <filesystem>
#include <assimp/postprocess.h>

namespace toaster
{
	Mesh::~Mesh()
	{
		destroy();
	}

	bool Mesh::loadFromFile(const std::string &filePath, gpu::GPUContext *gpuContext)
	{
		m_path = filePath;

		m_gpuContext = gpuContext;

		Assimp::Importer importer;

		constexpr uint32 importFlags = aiProcess_Triangulate |           // Convert polygons to triangles
									   aiProcess_GenNormals |            // Generate normals if not present
									   aiProcess_FlipUVs |               // Flip UVs for Vulkan coordinate system
									   aiProcess_CalcTangentSpace |      // Calculate tangents and bitangents
									   aiProcess_JoinIdenticalVertices | // Optimize vertex count
									   aiProcess_OptimizeMeshes;         // Reduce draw call count

		const aiScene *scene = importer.ReadFile(filePath, importFlags);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			LOG_ERROR("Assimp error loading '{}': {}", filePath, importer.GetErrorString());
			return false;
		}

		LOG_INFO("Loading mesh: {}", filePath);
		LOG_INFO("  Meshes: {}", scene->mNumMeshes);
		LOG_INFO("  Materials: {}", scene->mNumMaterials);

		// Clear any existing data
		m_vertices.clear();
		m_indices.clear();
		m_subMeshes.clear();

		// Process all nodes recursively
		processNode(scene->mRootNode, scene);

		if (m_vertices.empty() || m_indices.empty())
		{
			LOG_ERROR("Mesh '{}' has no vertex or index data", filePath);
			return false;
		}

		LOG_INFO("  Total vertices: {}", m_vertices.size());
		LOG_INFO("  Total indices: {}", m_indices.size());
		LOG_INFO("  SubMeshes: {}", m_subMeshes.size());

		// Create GPU buffers
		createVertexBuffer();
		createIndexBuffer();

		m_isLoaded = true;
		return true;
	}

	void Mesh::destroy()
	{
		if (!m_gpuContext)
			return;

		m_vertices.clear();
		m_indices.clear();
		m_subMeshes.clear();
		m_isLoaded = false;
	}

	void Mesh::processNode(aiNode *node, const aiScene *scene)
	{
		// Process all meshes in this node
		for (uint32 i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
			processMesh(mesh, scene);
		}

		// Recursively process child nodes
		for (uint32 i = 0; i < node->mNumChildren; i++)
		{
			processNode(node->mChildren[i], scene);
		}
	}

	void Mesh::processMesh(aiMesh *mesh, const aiScene *scene)
	{
		SubMesh subMesh{};
		subMesh.vertexOffset = static_cast<uint32>(m_vertices.size());
		subMesh.indexOffset  = static_cast<uint32>(m_indices.size());

		// Process vertices
		for (uint32 i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex{};

			// Position
			vertex.position.x = mesh->mVertices[i].x;
			vertex.position.y = mesh->mVertices[i].y;
			vertex.position.z = mesh->mVertices[i].z;

			// Normal
			if (mesh->HasNormals())
			{
				vertex.normal.x = mesh->mNormals[i].x;
				vertex.normal.y = mesh->mNormals[i].y;
				vertex.normal.z = mesh->mNormals[i].z;
			}
			else
			{
				vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
			}

			// Texture coordinates (use first UV channel if available)
			if (mesh->mTextureCoords[0])
			{
				vertex.texCoord.x = mesh->mTextureCoords[0][i].x;
				vertex.texCoord.y = mesh->mTextureCoords[0][i].y;
			}
			else
			{
				vertex.texCoord = glm::vec2(0.0f, 0.0f);
			}

			m_vertices.push_back(vertex);
		}

		// Process indices
		for (uint32 i = 0; i < mesh->mNumFaces; i++)
		{
			const aiFace &face = mesh->mFaces[i];
			for (uint32 j = 0; j < face.mNumIndices; j++)
			{
				// Add vertex offset for multi-mesh support
				m_indices.push_back(face.mIndices[j] + subMesh.vertexOffset);
			}
		}

		subMesh.indexCount = static_cast<uint32>(m_indices.size()) - subMesh.indexOffset;
		m_subMeshes.push_back(subMesh);
	}

	void Mesh::createVertexBuffer()
	{
		const vk::DeviceSize bufferSize = m_vertices.size() * sizeof(Vertex);

		m_vertexBuffer = std::make_shared<gpu::VertexBuffer>(m_gpuContext, m_vertices.data(), bufferSize);
	}

	void Mesh::createIndexBuffer()
	{
		const vk::DeviceSize bufferSize = m_indices.size() * sizeof(uint32);

		m_indexBuffer = std::make_shared<gpu::IndexBuffer>(m_gpuContext, m_indices.data(), bufferSize);
	}
}
