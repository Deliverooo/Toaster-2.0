#include "vk_mesh.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace toaster::gpu
{
	static constexpr uint32 s_MeshImportFlags = aiProcess_CalcTangentSpace // Create binormals/tangents just in case
												| aiProcess_Triangulate    // Make sure we're triangles
												| aiProcess_SortByPType    // Split meshes by primitive type
												| aiProcess_GenNormals     // Make sure we have legit normals
												| aiProcess_GenUVCoords    // Convert UVs if required
												//		| aiProcess_OptimizeGraph
												| aiProcess_OptimizeMeshes // Batch draws where possible
												| aiProcess_JoinIdenticalVertices | aiProcess_LimitBoneWeights
												// If more than N (=4) bone weights, discard least influencing bones and renormalise sum to 1
												| aiProcess_ValidateDataStructure // Validation
												| aiProcess_GlobalScale           // e.g. convert cm to m for fbx import (and other formats where cm is native)
	;

	VKMesh::VKMesh(VKGPUContext *p_ctx, const io::filesystem::Path &p_path) : m_ctx(p_ctx), m_path(p_path)
	{
		Assimp::Importer importer;

		const aiScene *scene = importer.ReadFile(p_path.string(), s_MeshImportFlags);
		if (!scene)
		{
			LOG_ERROR("Mesh", "Failed to load mesh file: {0}", p_path.string());
			TST_ASSERT(false);
		}



		if (scene->HasMeshes())
		{
			uint32 vertex_count{0u};
			uint32 index_count{0u};

			for (uint32 i{0u}; i < scene->mNumMeshes; ++i)
			{
				aiMesh *ai_mesh = scene->mMeshes[i];

				if (!ai_mesh->HasPositions())
					LOG_WARN("Mesh index {0} with name '{1}' has no vertex positions - skipping import!", i, ai_mesh->mName.C_Str());
				if (!ai_mesh->HasNormals())
					LOG_WARN("Mesh index {0} with name '{1}' has no vertex normals, and they could not be computed - skipping import!", i, ai_mesh->mName.C_Str());

				bool skip = !ai_mesh->HasPositions() || !ai_mesh->HasNormals();

				if (skip)
					continue;

				vertex_count += ai_mesh->mNumVertices;
				index_count  += skip ? 0 : ai_mesh->mNumFaces * 3;

				for (uint32 j{0u}; j < ai_mesh->mNumVertices; ++j)
				{
					MeshVertex vertex{};
					vertex.position = {ai_mesh->mVertices[j].x, ai_mesh->mVertices[j].y, ai_mesh->mVertices[j].z};
					vertex.normal   = {ai_mesh->mNormals[j].x, ai_mesh->mNormals[j].y, ai_mesh->mNormals[j].z};

					if (ai_mesh->HasTangentsAndBitangents())
					{
						vertex.tangent   = {ai_mesh->mTangents[j].x, ai_mesh->mTangents[j].y, ai_mesh->mTangents[j].z};
						vertex.bitangent = {ai_mesh->mBitangents[j].x, ai_mesh->mBitangents[j].y, ai_mesh->mBitangents[j].z};
					}
					if (ai_mesh->HasTextureCoords(0))
					{
						vertex.texCoord = {ai_mesh->mTextureCoords[0][j].x, ai_mesh->mTextureCoords[0][j].y};
					}

					m_vertices.emplace_back(vertex);
				}

				for (uint32 j{0u}; j < ai_mesh->mNumFaces; ++j)
				{
					m_indices.emplace_back(ai_mesh->mFaces[j].mIndices[0]);
					m_indices.emplace_back(ai_mesh->mFaces[j].mIndices[1]);
					m_indices.emplace_back(ai_mesh->mFaces[j].mIndices[2]);
				}
			}
		}

		const vk::DeviceSize vertex_buffer_size{sizeof(MeshVertex) * m_vertices.size()};
		m_vertexBuffer = make_reference<VKVertexBuffer>(m_ctx, (void *) m_vertices.data(), vertex_buffer_size);

		const vk::DeviceSize index_buffer_size{sizeof(uint16) * m_indices.size()};
		m_indexBuffer = make_reference<VKIndexBuffer>(m_ctx, (void *) m_indices.data(), index_buffer_size);

		if (scene->HasMaterials())
		{
			for (uint32 i{0u}; i < scene->mNumMaterials; ++i)
			{
				auto ai_material      = scene->mMaterials[i];
				auto ai_material_name = ai_material->GetName();

				aiString ai_tex_path;

				glm::vec3 albedo_colour{0.8f};
				aiColor3D ai_colour;
				if (ai_material->Get(AI_MATKEY_COLOR_DIFFUSE, ai_colour) == AI_SUCCESS)
					albedo_colour = {ai_colour.r, ai_colour.g, ai_colour.b};

				float32 roughness;
				if (ai_material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) != aiReturn_SUCCESS)
					roughness = 0.4f;

				float32 metalness;
				if (ai_material->Get(AI_MATKEY_REFLECTIVITY, metalness) != aiReturn_SUCCESS)
					metalness = 0.0f;

				if (metalness < 0.9f)
					metalness = 0.0f;
				else
					metalness = 1.0f;

				LOG_TRACE("\tCOLOUR = {}, {}, {}", ai_colour.r, ai_colour.g, ai_colour.b);
				LOG_TRACE("\tROUGHNESS = {}", roughness);
				LOG_TRACE("\tMETALNESS = {}", metalness);

				m_roughness = roughness;

				bool has_albedo_map = ai_material->GetTexture(AI_MATKEY_BASE_COLOR_TEXTURE, &ai_tex_path) == AI_SUCCESS;
				if (!has_albedo_map)
					has_albedo_map = ai_material->GetTexture(aiTextureType_DIFFUSE, 0, &ai_tex_path) == AI_SUCCESS;

				if (has_albedo_map)
				{
					auto parent_path  = p_path.parent_path();
					auto texture_path = parent_path / ai_tex_path.C_Str();
					if (!io::filesystem::exists(texture_path))
					{
						LOG_TRACE("\tAlbedo map path = {} --> NOT FOUND", texture_path.string());
						texture_path = parent_path / texture_path.filename();
					}
					LOG_TRACE("\tAlbedo map path = {}{}", texture_path.string(), std::filesystem::exists(texture_path) ? "" : " --> NOT FOUND");

					TextureSpecInfo texture_spec_info{};
					texture_spec_info.generateMips = false;
					m_albedoMap = make_reference<VKTexture2D>(m_ctx, texture_spec_info, texture_path);
				}
				else
				{
					TST_ASSERT(false);
				}
			}
		}
	}

	const RefPtr<VKVertexBuffer> &VKMesh::getVertexBuffer() const
	{
		return m_vertexBuffer;
	}

	const RefPtr<VKIndexBuffer> &VKMesh::getIndexBuffer() const
	{
		return m_indexBuffer;
	}

	const RefPtr<VKTexture2D> &VKMesh::getAlbedoMap() const
	{
		return m_albedoMap;
	}

	float32 VKMesh::getRoughness() const
	{
		return m_roughness;
	}

	const std::vector<MeshVertex> &VKMesh::getVertices() const
	{
		return m_vertices;
	}

	const std::vector<uint16> &VKMesh::getIndices() const
	{
		return m_indices;
	}
}
