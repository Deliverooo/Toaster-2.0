#include "mesh.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "toast_lib/logging.hpp"

#include "globals.hpp"

namespace toaster
{
	glm::mat4 Mat4FromAssimpMat4(const aiMatrix4x4 &matrix)
	{
		glm::mat4 result;
		//the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
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

	static const uint32_t s_MeshImportFlags = aiProcess_CalcTangentSpace |     // Create binormals/tangents just in case
											  aiProcess_Triangulate |          // Make sure we're triangles
											  aiProcess_SortByPType |          // Split meshes by primitive type
											  aiProcess_GenNormals |           // Make sure we have legit normals
											  aiProcess_GenUVCoords |          // Convert UVs if required
											  aiProcess_OptimizeMeshes |       // Batch draws where possible
											  aiProcess_ValidateDataStructure; // Validation

	RefPtr<Mesh> Mesh::create(const std::vector<MeshVertex> &p_vertices, const std::vector<uint32> &p_indices)
	{
		return make_reference<Mesh>(p_vertices, p_indices);
	}

	Mesh::Mesh(const std::vector<MeshVertex> &p_vertices, const std::vector<uint32> &p_indices) : m_vertices(p_vertices), m_indices(p_indices)
	{
		m_vertexBuffer = gpu::IVertexBuffer::create(m_vertices.data(), m_vertices.size() * sizeof(MeshVertex));
		const auto vbl = gpu::VertexBufferLayout{
			{gpu::EShaderDataType::eFloat3, "a_Position"},
			{gpu::EShaderDataType::eFloat3, "a_Normal"},
			{gpu::EShaderDataType::eFloat3, "a_Tangent"},
			{gpu::EShaderDataType::eFloat3, "a_Bitangent"},
			{gpu::EShaderDataType::eFloat2, "a_TexCoord"}
		};
		m_vertexBuffer->setLayout(vbl);

		m_indexBuffer = gpu::IIndexBuffer::create(m_indices.data(), m_indices.size());

		m_vertexArray = gpu::IVertexArray::create();
		m_vertexArray->addVertexBuffer(m_vertexBuffer);
		m_vertexArray->setIndexBuffer(m_indexBuffer);

		m_shader = Globals::shaderLibrary()->get("Mesh");
	}

	RefPtr<Mesh> Mesh::importFromFile(const io::filesystem::Path &p_path)
	{
		auto mesh = make_reference<Mesh>();

		Assimp::Importer importer;

		const aiScene *scene = importer.ReadFile(p_path.string(), s_MeshImportFlags);
		if (!scene /* || !scene->HasMeshes()*/) // note: scene can legit contain no meshes (e.g. it could contain an armature, an animation, and no skin (mesh)))
		{
			LOG_ERROR("Mesh", "Failed to load mesh file: {0}", p_path.string());
			return nullptr;
		}

		if (scene->HasMeshes())
		{
			uint32 vertex_count{0u};
			uint32 index_count{0u};

			mesh->m_submeshes.reserve(scene->mNumMeshes);

			for (uint32 i{0u}; i < scene->mNumMeshes; ++i)
			{
				aiMesh *ai_mesh = scene->mMeshes[i];

				if (!ai_mesh->HasPositions())
				{
					LOG_WARN("Mesh index {0} with name '{1}' has no vertex positions - skipping import!", i, ai_mesh->mName.C_Str());
				}
				if (!ai_mesh->HasNormals())
				{
					LOG_WARN("Mesh index {0} with name '{1}' has no vertex normals, and they could not be computed - skipping import!", i, ai_mesh->mName.C_Str());
				}

				bool skip = !ai_mesh->HasPositions() || !ai_mesh->HasNormals();

				Submesh &submesh      = mesh->m_submeshes.emplace_back();
				submesh.baseVertex    = vertex_count;
				submesh.baseIndex     = index_count;
				submesh.materialIndex = ai_mesh->mMaterialIndex;
				submesh.vertexCount   = skip ? 0 : ai_mesh->mNumVertices;
				submesh.indexCount    = skip ? 0 : ai_mesh->mNumFaces * 3;
				submesh.meshName      = ai_mesh->mName.C_Str();

				if (skip)
					continue;

				vertex_count += ai_mesh->mNumVertices;
				index_count  += submesh.indexCount;

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

					mesh->m_vertices.push_back(vertex);
				}

				for (uint32 j{0u}; j < ai_mesh->mNumFaces; ++j)
				{
					mesh->m_indices.push_back(ai_mesh->mFaces[j].mIndices[0]);
					mesh->m_indices.push_back(ai_mesh->mFaces[j].mIndices[1]);
					mesh->m_indices.push_back(ai_mesh->mFaces[j].mIndices[2]);
				}
			}

			MeshNode &root_node = mesh->m_nodes.emplace_back();
			mesh->traverseNodes(scene->mRootNode, 0);
		}

		if (scene->HasMaterials())
		{
			LOG_TRACE("---- Materials - {} ----", p_path.string());

			mesh->m_materials.resize(scene->mNumMaterials);
			for (uint32 i{0u}; i < scene->mNumMaterials; ++i)
			{
				auto ai_material      = scene->mMaterials[i];
				auto ai_material_name = ai_material->GetName();

				auto &material =mesh->m_materials[i];
				material = Material::create(Globals::shaderLibrary()->get("Mesh"));

				LOG_TRACE("\t{} (Index = {})", ai_material_name.data, i);

				aiString ai_tex_path;

				glm::vec3 albedo_colour{0.8f};
				aiColor3D ai_colour;
				if (ai_material->Get(AI_MATKEY_COLOR_DIFFUSE, ai_colour) == AI_SUCCESS)
					albedo_colour = {ai_colour.r, ai_colour.g, ai_colour.b};

				material->set("u_AlbedoColour", albedo_colour);

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

				material->set("u_Roughness", roughness);
				material->set("u_Metalness", metalness);

				LOG_TRACE("\tCOLOUR = {}, {}, {}", ai_colour.r, ai_colour.g, ai_colour.b);
				LOG_TRACE("\tROUGHNESS = {}", roughness);
				LOG_TRACE("\tMETALNESS = {}", metalness);

				bool has_albedo_map = ai_material->GetTexture(AI_MATKEY_BASE_COLOR_TEXTURE, &ai_tex_path) == AI_SUCCESS;
				if (!has_albedo_map)
				{
					has_albedo_map = ai_material->GetTexture(aiTextureType_DIFFUSE, 0, &ai_tex_path) == AI_SUCCESS;
				}

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

					auto albedo_map = gpu::ITexture2D::create(texture_path);

					material->set("u_AlbedoMap", albedo_map);
					material->set("u_AlbedoColour", glm::vec3{1.0f});
				}
				else
				{
					auto map = gpu::ITexture2D::create("error");

					material->set("u_AlbedoMap", map);
				}

				mesh->m_materials[i] = material;
			}
		}

		mesh->m_vertexBuffer = gpu::IVertexBuffer::create(mesh->m_vertices.data(), mesh->m_vertices.size() * sizeof(MeshVertex));
		const auto vbl       = gpu::VertexBufferLayout{
			{gpu::EShaderDataType::eFloat3, "a_Position"},
			{gpu::EShaderDataType::eFloat3, "a_Normal"},
			{gpu::EShaderDataType::eFloat3, "a_Tangent"},
			{gpu::EShaderDataType::eFloat3, "a_Bitangent"},
			{gpu::EShaderDataType::eFloat2, "a_TexCoord"}
		};
		mesh->m_vertexBuffer->setLayout(vbl);

		mesh->m_indexBuffer = gpu::IIndexBuffer::create(mesh->m_indices.data(), mesh->m_indices.size());

		mesh->m_vertexArray = gpu::IVertexArray::create();
		mesh->m_vertexArray->addVertexBuffer(mesh->m_vertexBuffer);
		mesh->m_vertexArray->setIndexBuffer(mesh->m_indexBuffer);

		return mesh;
	}

	RefPtr<gpu::IVertexBuffer> Mesh::getVertexBuffer() const
	{
		return m_vertexBuffer;
	}

	RefPtr<gpu::IIndexBuffer> Mesh::getIndexBuffer() const
	{
		return m_indexBuffer;
	}

	RefPtr<gpu::IVertexArray> Mesh::getVertexArray() const
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

	const std::vector<Submesh> &Mesh::getSubmeshes() const
	{
		return m_submeshes;
	}

	const RefPtr<Material> &Mesh::getMaterial(uint32 p_index) const
	{
		TST_ASSERT_MSG(p_index < m_materials.size(), "Out of range!");
		return m_materials[p_index];
	}

	std::vector<RefPtr<Material> > Mesh::getMaterials() const
	{
		return m_materials;
	}

	void Mesh::traverseNodes(void *p_assimp_node, uint32 p_node_index, const glm::mat4 &p_parent_transform, uint32 p_level)
	{
		aiNode *ai_node = static_cast<aiNode *>(p_assimp_node);

		MeshNode &node = m_nodes[p_node_index];
		node.name      = ai_node->mName.C_Str();
		node.transform = Mat4FromAssimpMat4(ai_node->mTransformation);

		glm::mat4 transform = p_parent_transform * node.transform;
		for (uint32 i{0u}; i < ai_node->mNumMeshes; ++i)
		{
			uint32 submesh_index = ai_node->mMeshes[i];

			auto &submesh          = m_submeshes[submesh_index];
			submesh.nodeName       = ai_node->mName.C_Str();
			submesh.transform      = transform;
			submesh.localTransform = node.transform;

			node.submeshes.push_back(submesh_index);
		}

		uint32 parent_node_index = m_nodes.size() - 1u;
		node.children.resize(ai_node->mNumChildren);
		for (uint32 i{0u}; i < ai_node->mNumChildren; ++i)
		{
			MeshNode &child                   = m_nodes.emplace_back();
			uint32    child_index             = m_nodes.size() - 1u;
			child.parent                      = parent_node_index;
			m_nodes[p_node_index].children[i] = child_index;
			traverseNodes(ai_node->mChildren[i], child_index, transform, p_level + 1);
		}
	}
}
