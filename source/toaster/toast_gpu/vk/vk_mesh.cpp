#include "vk_mesh.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "vk_gpu_context.hpp"

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

	auto mat4FromAIMatrix4x4(const aiMatrix4x4 &matrix) -> glm::mat4
	{
		glm::mat4 result;
		result[0][0] = matrix.a1;
		result[1][0] = matrix.a2;
		result[2][0] = matrix.a3;
		result[3][0] = matrix.a4;
		result[0][1] = matrix.b1;
		result[1][1] = matrix.b2;
		result[2][1] = matrix.b3;
		result[3][1] = matrix.b4;
		result[0][2] = matrix.c1;
		result[1][2] = matrix.c2;
		result[2][2] = matrix.c3;
		result[3][2] = matrix.c4;
		result[0][3] = matrix.d1;
		result[1][3] = matrix.d2;
		result[2][3] = matrix.d3;
		result[3][3] = matrix.d4;
		return result;
	}

	VKMesh::VKMesh(VKGPUContext *p_ctx, const io::filesystem::Path &p_path, const RefPtr<VKShader> &p_shader) : m_ctx(p_ctx), m_path(p_path)
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

			for (uint32 mesh_i{0u}; mesh_i < scene->mNumMeshes; ++mesh_i)
			{
				aiMesh *ai_mesh = scene->mMeshes[mesh_i];

				if (!ai_mesh->HasPositions())
					LOG_WARN("Mesh index {0} with name '{1}' has no vertex positions - skipping import!", mesh_i, ai_mesh->mName.C_Str());
				if (!ai_mesh->HasNormals())
					LOG_WARN("Mesh index {0} with name '{1}' has no vertex normals, and they could not be computed - skipping import!", mesh_i, ai_mesh->mName.C_Str());

				bool skip = !ai_mesh->HasPositions() || !ai_mesh->HasNormals();

				Submesh &submesh      = m_submeshes.emplace_back();
				submesh.baseVertex    = vertex_count;
				submesh.baseIndex     = index_count;
				submesh.materialIndex = ai_mesh->mMaterialIndex;
				submesh.vertexCount   = skip ? 0 : ai_mesh->mNumVertices;
				submesh.indexCount    = skip ? 0 : ai_mesh->mNumFaces * 3;
				submesh.name          = ai_mesh->mName.C_Str();

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
						vertex.texCoord = {ai_mesh->mTextureCoords[0][j].x, ai_mesh->mTextureCoords[0][j].y};

					m_vertices.emplace_back(vertex);
				}

				for (uint32 j{0u}; j < ai_mesh->mNumFaces; ++j)
				{
					m_indices.emplace_back(ai_mesh->mFaces[j].mIndices[0]);
					m_indices.emplace_back(ai_mesh->mFaces[j].mIndices[1]);
					m_indices.emplace_back(ai_mesh->mFaces[j].mIndices[2]);
				}
			}

			MeshNode &rootNode = m_nodes.emplace_back();
			_traverseNodes(scene->mRootNode, 0, glm::mat4{1.0f}, 0);
		}

		if (scene->HasMaterials())
		{
			for (uint32 i{0u}; i < scene->mNumMaterials; ++i)
			{
				auto  ai_material      = scene->mMaterials[i];
				auto  ai_material_name = ai_material->GetName();
				auto &material{m_materials.emplace_back(m_ctx->alloc<VKMaterial>(p_shader))};

				aiString ai_tex_path;

				glm::vec3 albedo_colour{0.8f};
				aiColor3D ai_colour;
				if (ai_material->Get(AI_MATKEY_COLOR_DIFFUSE, ai_colour) == AI_SUCCESS)
					albedo_colour = {ai_colour.r, ai_colour.g, ai_colour.b};
				material->set("u_Material.albedoColour", albedo_colour);

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

				// m_roughness = roughness;

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
					texture_spec_info.generateMips = true;
					material->set("u_Texture", m_ctx->alloc<VKTexture2D>(texture_spec_info, texture_path));
					material->set("u_Material.albedoColour", glm::vec3{1.0f});
				}
				else
					LOG_WARN("Mesh material does not have an albedo map");
			}
		}
		else
			m_materials.emplace_back(m_ctx->alloc<VKMaterial>(p_shader));

		const vk::DeviceSize vertex_buffer_size{sizeof(MeshVertex) * m_vertices.size()};
		m_vertexBuffer = make_reference<VKVertexBuffer>(m_ctx, (void *) m_vertices.data(), vertex_buffer_size);

		const vk::DeviceSize index_buffer_size{sizeof(uint16) * m_indices.size()};
		m_indexBuffer = make_reference<VKIndexBuffer>(m_ctx, (void *) m_indices.data(), index_buffer_size);
	}

	auto VKMesh::getContext() const -> VKGPUContext *
	{
		return m_ctx;
	}

	auto VKMesh::getVertexBuffer() const -> const RefPtr<VKVertexBuffer> &
	{
		return m_vertexBuffer;
	}

	auto VKMesh::getIndexBuffer() const -> const RefPtr<VKIndexBuffer> &
	{
		return m_indexBuffer;
	}

	auto VKMesh::getMaterials() const -> const std::vector<RefPtr<VKMaterial> > &
	{
		return m_materials;
	}

	auto VKMesh::getSubmeshes() const -> const std::vector<Submesh> &
	{
		return m_submeshes;
	}

	auto VKMesh::getVertices() const -> const std::vector<MeshVertex> &
	{
		return m_vertices;
	}

	auto VKMesh::getIndices() const -> const std::vector<uint16> &
	{
		return m_indices;
	}

	auto VKMesh::_traverseNodes(void *p_assimp_node, uint32 p_node_index, const glm::mat4 &p_parent_transform, uint32 p_level) -> void
	{
		auto ai_node = static_cast<aiNode *>(p_assimp_node);

		MeshNode &node{m_nodes[p_node_index]};
		node.name           = ai_node->mName.C_Str();
		node.localTransform = mat4FromAIMatrix4x4(ai_node->mTransformation);

		glm::mat4 transform{p_parent_transform * node.localTransform};
		for (uint32 i{0u}; i < ai_node->mNumMeshes; ++i)
		{
			uint32_t submesh_index = ai_node->mMeshes[i];
			auto &   submesh       = m_submeshes[submesh_index];
			submesh.name           = ai_node->mName.C_Str();
			submesh.transform      = transform;
			submesh.localTransform = node.localTransform;

			node.submeshes.emplace_back(submesh_index);
		}

		uint32 parent_node_index = static_cast<uint32>(m_nodes.size()) - 1;
		node.children.resize(ai_node->mNumChildren);
		for (uint32 i{0u}; i < ai_node->mNumChildren; ++i)
		{
			MeshNode &child                   = m_nodes.emplace_back();
			uint32    child_index             = static_cast<uint32>(m_nodes.size()) - 1;
			child.parent                      = parent_node_index;
			m_nodes[p_node_index].children[i] = child_index;
			_traverseNodes(ai_node->mChildren[i], child_index, transform, p_level + 1);
		}
	}
}
