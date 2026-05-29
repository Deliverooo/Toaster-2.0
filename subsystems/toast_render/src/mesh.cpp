#include "toast_render/mesh.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "toast_render/globals.hpp"

namespace toaster::render
{
	static constexpr uint32 s_MeshImportFlags{
		aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_GenNormals | aiProcess_GenUVCoords | aiProcess_OptimizeMeshes |
		aiProcess_JoinIdenticalVertices | aiProcess_LimitBoneWeights | aiProcess_ValidateDataStructure | aiProcess_GlobalScale | aiProcess_FlipUVs
	};

	auto mat4FromAIMatrix4x4(const aiMatrix4x4 &matrix) -> tsm::float4x4
	{
		tsm::float4x4 result;
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

	MaterialList::MaterialList(RenderContext *p_render_ctx) : m_renderCtx(p_render_ctx)
	{
	}

	auto MaterialList::addMaterial(uint32 p_index, const gpu::ShaderHandle &p_shader, const String &p_name) -> MeshMaterialData &
	{
		auto &mat{m_materialDatas[p_index]};
		mat.material = m_renderCtx->create<Material>(p_shader, p_name);
		mat.name     = p_name;
		return mat;
	}

	auto MaterialList::hasMaterial(uint32 p_index) const -> bool
	{
		return m_materialDatas.contains(p_index);
	}

	auto MaterialList::getMaterial(uint32 p_index) -> MeshMaterialData &
	{
		TST_PERMA_ASSERT(hasMaterial(p_index));
		return m_materialDatas[p_index];
	}

	auto MaterialList::getMaterial(uint32 p_index) const -> const MeshMaterialData &
	{
		TST_PERMA_ASSERT(hasMaterial(p_index));
		return m_materialDatas.at(p_index);
	}

	MeshData::MeshData(RenderContext *p_render_ctx, const io::filesystem::Path &p_path) : m_renderCtx(p_render_ctx), m_path(p_path), m_materials(p_render_ctx)
	{
		Assimp::Importer importer;

		const aiScene *scene = importer.ReadFile(p_path.string(), s_MeshImportFlags);
		if (!scene)
		{
			LOG_ERROR("Failed to load mesh file: {0}", p_path.string());
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
				index_count  += submesh.indexCount;

				for (uint32 i{0u}; i < ai_mesh->mNumVertices; ++i)
				{
					MeshVertex vertex{};
					vertex.position = {ai_mesh->mVertices[i].x, ai_mesh->mVertices[i].y, ai_mesh->mVertices[i].z};
					vertex.normal   = {ai_mesh->mNormals[i].x, ai_mesh->mNormals[i].y, ai_mesh->mNormals[i].z};

					if (ai_mesh->HasTangentsAndBitangents())
					{
						vertex.tangent   = {ai_mesh->mTangents[i].x, ai_mesh->mTangents[i].y, ai_mesh->mTangents[i].z};
						vertex.bitangent = {ai_mesh->mBitangents[i].x, ai_mesh->mBitangents[i].y, ai_mesh->mBitangents[i].z};
					}
					if (ai_mesh->HasTextureCoords(0))
						vertex.texCoord = {ai_mesh->mTextureCoords[0][i].x, ai_mesh->mTextureCoords[0][i].y};

					m_vertices.emplace_back(vertex);
				}

				for (uint32 i{0u}; i < ai_mesh->mNumFaces; ++i)
				{
					TST_ASSERT_MSG(ai_mesh->mFaces[i].mNumIndices == 3, "Must have 3 indices!");
					m_indices.emplace_back(ai_mesh->mFaces[i].mIndices[0]);
					m_indices.emplace_back(ai_mesh->mFaces[i].mIndices[1]);
					m_indices.emplace_back(ai_mesh->mFaces[i].mIndices[2]);
				}
			}

			MeshNode &rootNode = m_nodes.emplace_back();
			(void) rootNode;
			_traverseNodes(scene->mRootNode, 0, tsm::float4x4{1.0f}, 0);
		}

		if (scene->HasMaterials())
		{
			for (uint32 i{0u}; i < scene->mNumMaterials; ++i)
			{
				auto ai_material = scene->mMaterials[i];
				_createMaterial(ai_material, i, p_path);
			}
		}
		else
			m_materials.addMaterial(0, p_render_ctx->getGlobals()->shaderLibrary().get("Geometry"), "Default");

		m_vertexBuffer = m_renderCtx->createVertexBuffer(m_vertices);
		m_indexBuffer  = m_renderCtx->createIndexBuffer(m_indices);
	}

	MeshData::MeshData(RenderContext *p_render_ctx, const io::filesystem::Path &p_path, const gpu::ShaderHandle &p_shader) : m_renderCtx(p_render_ctx), m_path(p_path),
																															 m_materials(p_render_ctx)
	{
		Assimp::Importer importer;

		const aiScene *scene = importer.ReadFile(p_path.string(), s_MeshImportFlags);
		if (!scene)
		{
			LOG_ERROR("Failed to load mesh file: {0}", p_path.string());
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
				index_count  += submesh.indexCount;

				for (uint32 i{0u}; i < ai_mesh->mNumVertices; ++i)
				{
					MeshVertex vertex{};
					vertex.position = {ai_mesh->mVertices[i].x, ai_mesh->mVertices[i].y, ai_mesh->mVertices[i].z};
					vertex.normal   = {ai_mesh->mNormals[i].x, ai_mesh->mNormals[i].y, ai_mesh->mNormals[i].z};

					if (ai_mesh->HasTangentsAndBitangents())
					{
						vertex.tangent   = {ai_mesh->mTangents[i].x, ai_mesh->mTangents[i].y, ai_mesh->mTangents[i].z};
						vertex.bitangent = {ai_mesh->mBitangents[i].x, ai_mesh->mBitangents[i].y, ai_mesh->mBitangents[i].z};
					}
					if (ai_mesh->HasTextureCoords(0))
						vertex.texCoord = {ai_mesh->mTextureCoords[0][i].x, ai_mesh->mTextureCoords[0][i].y};

					m_vertices.emplace_back(vertex);
				}

				for (uint32 i{0u}; i < ai_mesh->mNumFaces; ++i)
				{
					TST_ASSERT_MSG(ai_mesh->mFaces[i].mNumIndices == 3, "Must have 3 indices!");
					m_indices.emplace_back(ai_mesh->mFaces[i].mIndices[0]);
					m_indices.emplace_back(ai_mesh->mFaces[i].mIndices[1]);
					m_indices.emplace_back(ai_mesh->mFaces[i].mIndices[2]);
				}
			}

			MeshNode &rootNode = m_nodes.emplace_back();
			(void) rootNode;
			_traverseNodes(scene->mRootNode, 0, tsm::float4x4{1.0f}, 0);
		}

		if (scene->HasMaterials())
		{
			for (uint32 i{0u}; i < scene->mNumMaterials; ++i)
			{
				auto ai_material = scene->mMaterials[i];
				_createMaterial(ai_material, i, p_path);
			}
		}
		else
			m_materials.addMaterial(0, p_shader, "Default");

		m_vertexBuffer = m_renderCtx->createVertexBuffer(m_vertices);
		m_indexBuffer  = m_renderCtx->createIndexBuffer(m_indices);
	}

	auto MeshData::getVertexBuffer() const -> const gpu::VertexBufferHandle &
	{
		return m_vertexBuffer;
	}

	auto MeshData::getIndexBuffer() const -> const gpu::IndexBufferHandle &
	{
		return m_indexBuffer;
	}

	auto MeshData::getMaterials() -> MaterialList &
	{
		return m_materials;
	}

	auto MeshData::getMaterials() const -> const MaterialList &
	{
		return m_materials;
	}

	auto MeshData::getSubmeshes() const -> const std::vector<Submesh> &
	{
		return m_submeshes;
	}

	auto MeshData::getVertices() const -> const std::vector<MeshVertex> &
	{
		return m_vertices;
	}

	auto MeshData::getIndices() const -> const std::vector<uint32> &
	{
		return m_indices;
	}

	auto MeshData::getFilepath() const -> const io::filesystem::Path &
	{
		return m_path;
	}

	auto MeshData::_traverseNodes(void *p_assimp_node, uint32 p_node_index, const tsm::float4x4 &p_parent_transform, uint32 p_level) -> void
	{
		auto ai_node = static_cast<aiNode *>(p_assimp_node);

		MeshNode &node{m_nodes[p_node_index]};
		node.name           = ai_node->mName.C_Str();
		node.localTransform = mat4FromAIMatrix4x4(ai_node->mTransformation);

		tsm::float4x4 transform{p_parent_transform * node.localTransform};
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

	auto MeshData::_createMaterial(void *p_mat, uint32 p_mat_index, const io::filesystem::Path &p_parent_path) -> void
	{
		auto ai_mat{static_cast<aiMaterial *>(p_mat)};

		String material_name{ai_mat->GetName().C_Str()};
		LOG_INFO("Creating new material: {}", material_name);

		auto &mat_data{m_materials.addMaterial(p_mat_index, m_renderCtx->getGlobals()->shaderLibrary().get("Geometry"), material_name)};
		auto &material{mat_data.material};

		// Albedo/base colour
		{
			tsm::float3 albedo_colour{0.8f};
			if (aiColor3D ai_colour; ai_mat->Get(AI_MATKEY_COLOR_DIFFUSE, ai_colour) == AI_SUCCESS)
				albedo_colour = {ai_colour.r, ai_colour.g, ai_colour.b};
			material->set("u_Material.albedoColour", albedo_colour);
			LOG_TRACE("\tAlbedo colour: {}", albedo_colour);
		}
		// Roughness
		{
			float32 roughness{0.4f};
			ai_mat->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);
			material->set("u_Material.roughness", roughness);
			LOG_TRACE("\tRoughness: {}", roughness);
		}
		// Metalness
		{
			float32 metalness{0.0f};
			ai_mat->Get(AI_MATKEY_REFLECTIVITY, metalness);
			material->set("u_Material.metalness", metalness);
			LOG_TRACE("\tMetalness: {}", metalness);
		}

		auto get_path_and_create_texture_if_exists{
			[p_parent_path](const aiString &p_ai_path, const String &p_tex_name) -> std::optional<io::filesystem::Path>
			{
				const io::filesystem::Path texture_path{p_ai_path.C_Str()};
				io::filesystem::Path       tex_map_path{p_parent_path.parent_path() / texture_path};

				if (!io::filesystem::exists(tex_map_path))
				{
					tex_map_path = p_parent_path.parent_path() / texture_path.filename();
					if (!io::filesystem::exists(tex_map_path))
					{
						LOG_ERROR("\tFailed to find {} map at: {}", p_tex_name, tex_map_path);
						return std::nullopt;
					}
				}

				LOG_INFO("\tSuccessfully found {} map: {}", p_tex_name, tex_map_path);
				return tex_map_path;
			}
		};

		// Load albedo map
		{
			aiString ai_albedo_map_path;
			bool     has_albedo_map{ai_mat->GetTexture(AI_MATKEY_BASE_COLOR_TEXTURE, &ai_albedo_map_path) == AI_SUCCESS};
			if (!has_albedo_map)
				has_albedo_map = ai_mat->GetTexture(aiTextureType_DIFFUSE, 0, &ai_albedo_map_path) == AI_SUCCESS;

			// TODO: Embedded textures
			if (has_albedo_map)
			{
				auto albedo_map_path{get_path_and_create_texture_if_exists(ai_albedo_map_path, "albedo")};
				if (albedo_map_path.has_value())
				{
					gpu::TextureSpecInfo texture_spec_info{};
					texture_spec_info.generateMips = true;
					mat_data.setAlbedoMap(m_renderCtx->createGPU<gpu::VKTexture2D>(texture_spec_info, *albedo_map_path));
				}
			}
			else
			{
				LOG_WARN("\tMaterial '{}' does not have an albedo map", material_name);
			}
		}

		// Load normal map
		{
			// TODO: Embedded textures
			aiString ai_normal_map_path;
			if (ai_mat->GetTexture(aiTextureType_NORMALS, 0, &ai_normal_map_path) == AI_SUCCESS)
			{
				auto normal_map_path{get_path_and_create_texture_if_exists(ai_normal_map_path, "normal")};
				if (normal_map_path.has_value())
				{
					gpu::TextureSpecInfo texture_spec_info{};
					texture_spec_info.generateMips = true;
					mat_data.setNormalMap(m_renderCtx->createGPU<gpu::VKTexture2D>(texture_spec_info, *normal_map_path));
				}
				material->set<uint32>("u_Material.hasNormalMap", 1u);
			}
			else
			{
				material->set<uint32>("u_Material.hasNormalMap", 0u);
				LOG_WARN("\tMaterial '{}' does not have a normal map", material_name);
			}
		}
	}

	StaticMesh::StaticMesh(RenderContext *p_render_ctx, const RefPtr<MeshData> &p_mesh_data) : m_renderCtx(p_render_ctx), m_meshData(p_mesh_data)
	{
	}

	auto StaticMesh::getMeshData() const -> const RefPtr<MeshData> &
	{
		return m_meshData;
	}

	DynamicMesh::DynamicMesh(RenderContext *p_render_ctx, const RefPtr<MeshData> &p_mesh_data) : m_renderCtx(p_render_ctx), m_meshData(p_mesh_data)
	{
	}

	auto DynamicMesh::getMeshData() const -> const RefPtr<MeshData> &
	{
		return m_meshData;
	}
}
