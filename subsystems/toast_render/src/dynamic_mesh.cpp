#include "toast_render/dynamic_mesh.hpp"

#include <thread>

#include "assimp_common.hpp"
#include "toast_render/globals.hpp"

namespace toaster::render
{
	static auto aiShadingModeToString(aiShadingMode p_shading_mode) -> String
	{
		switch (p_shading_mode)
		{
			case aiShadingMode_Flat: return "Flat";
			case aiShadingMode_Gouraud: return "Gouraud";
			case aiShadingMode_Phong: return "Phong";
			case aiShadingMode_Blinn: return "Blinn";
			case aiShadingMode_Toon: return "Toon";
			case aiShadingMode_OrenNayar: return "OrenNayar";
			case aiShadingMode_Minnaert: return "Minnaert";
			case aiShadingMode_CookTorrance: return "CookTorrance";
			case aiShadingMode_Unlit: return "Unlit";
			case aiShadingMode_Fresnel: return "Fresnel";
			case aiShadingMode_PBR_BRDF: return "PBR_BRDF";
			case _aiShadingMode_Force32Bit: return "Force32Bit";
		}
		return "";
	}

	DynamicMesh::DynamicMesh(RenderContext &p_render_ctx, const io::filesystem::Path &p_path) : m_renderCtx(&p_render_ctx)
	{
		Assimp::Importer importer;

		const aiScene *scene = importer.ReadFile(p_path.string(), s_MeshImportFlags);
		if (!scene)
		{
			LOG_ERROR("Failed to load mesh file: {0}", p_path.string());
			TST_PERMA_ASSERT(false);
		}

		std::thread import_thread{[&]() -> void { importMeshFromScene(scene, meshData); }};

		if (scene->HasMaterials())
		{
			for (uint32 i{0u}; i < scene->mNumMaterials; ++i)
			{
				auto ai_material = scene->mMaterials[i];
				_createMaterial(ai_material, i, p_path);
			}
		}
		else
		{
			TST_PERMA_ASSERT(false);
		}

		import_thread.join();

		uint64              index_buffer_size{meshData.indices.size() * sizeof(uint32)};
		gpu::BufferSpecInfo staging_buffer_spec{};
		staging_buffer_spec.deviceLocal = false;
		staging_buffer_spec.usageFlags  = vk::BufferUsageFlagBits2::eTransferSrc;
		gpu::Buffer staging_buffer{*m_renderCtx->getGPUContext(), index_buffer_size, staging_buffer_spec};
		staging_buffer.setData(meshData.indices.data(), index_buffer_size);

		gpu::BufferSpecInfo index_buffer_spec_info;
		index_buffer_spec_info.deviceLocal = true;
		index_buffer_spec_info.usageFlags  = vk::BufferUsageFlagBits2::eTransferDst | vk::BufferUsageFlagBits2::eIndexBuffer;
		m_indexBuffer                      = m_renderCtx->createGPUUnique<gpu::Buffer>(index_buffer_size, index_buffer_spec_info);

		m_indexBuffer->copyFromBuffer(staging_buffer);

		vertexBufferSSBO = m_renderCtx->createUnique<StorageBuffer>(meshData.vertices, true);

		const auto pixel_shader{m_renderCtx->getGlobals()->getShader("Dynamic_Mesh_PS")};

		const auto reflection_data{reflection::reflectShader(*pixel_shader)};
		const auto material_struct{findMaterialDeclaration(reflection_data)};
		TST_PERMA_ASSERT(material_struct);

		// materialBufferSSBO   = m_renderCtx->createUnique<StorageBuffer>(meshData.materialsGPUData, false);
		// m_mappedMaterialData = materialBufferSSBO->getBuffer()->mapMemory(materialBufferSSBO->getBuffer()->getSize());
	}

	DynamicMesh::~DynamicMesh()
	{
		// materialBufferSSBO->getBuffer()->unmapMemory();
	}

	auto DynamicMesh::getIndexBuffer() const -> const gpu::Buffer &
	{
		return *m_indexBuffer;
	}

	auto DynamicMesh::_createMaterial(void *p_mat, uint32 p_mat_index, const io::filesystem::Path &p_parent_path) -> void
	{
		auto   ai_mat{static_cast<aiMaterial *>(p_mat)};
		String material_name{ai_mat->GetName().C_Str()};

		auto &material{m_materials.emplace_back()};

		aiShadingMode shading_mode;
		aiReturn      res{ai_mat->Get(AI_MATKEY_SHADING_MODEL, shading_mode)};

		LOG_INFO("Shading mode: {}", aiShadingModeToString(shading_mode));

		EMaterialType material_type{EMaterialType::ePBR};
		m_materialTypes.emplace_back(material_type);

		switch (material_type)
		{
			case EMaterialType::ePBR:
			{
				const auto &material_struct{m_renderCtx->getGlobals()->getShaderReflectionData("Dynamic_Mesh_PS").materialStruct};
				material = m_renderCtx->createRef<DynamicMaterial>(m_renderCtx->getGlobals()->getShader("Dynamic_Mesh_PS"), &material_struct, material_name);
				break;
			}
			case EMaterialType::eFlat:
			{
				material = nullptr;
				break;
			}
		}

		bool two_sided{false};
		if (ai_mat->Get(AI_MATKEY_TWOSIDED, two_sided) == AI_SUCCESS)
		{
			LOG_INFO("Two sided: {}", two_sided);
			if (two_sided)
				material->flags |= EMaterialPropertyFlags::eTwoSided;
		}

		bool wireframe{false};
		if (ai_mat->Get(AI_MATKEY_ENABLE_WIREFRAME, wireframe) == AI_SUCCESS)
		{
			LOG_INFO("Wireframe: {}", wireframe);
			if (wireframe)
				material->flags |= EMaterialPropertyFlags::eWireframe;
		}

		float32 opacity{1.0f};
		if (ai_mat->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS)
		{
			LOG_INFO("Opacity: {}", opacity);

			if (opacity < 1.0f)
				material->flags |= EMaterialPropertyFlags::eTransparent;
		}

		if (aiColor3D ai_colour; ai_mat->Get(AI_MATKEY_COLOR_DIFFUSE, ai_colour) == AI_SUCCESS)
			material->set("albedoColour", tsm::float4{ai_colour.r, ai_colour.g, ai_colour.b, 1.0f});
		else
			material->set("albedoColour", tsm::float4{1.0f});

		if (float32 roughness; ai_mat->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == AI_SUCCESS)
			material->set("roughness", roughness);
		else
			material->set("roughness", 0.0f);

		if (float32 metalness; ai_mat->Get(AI_MATKEY_REFLECTIVITY, metalness) == AI_SUCCESS)
			material->set("metalness", metalness);
		else
			material->set("metalness", 0.0f);

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

				// LOG_INFO("\tSuccessfully found {} map: {}", p_tex_name, tex_map_path);
				return tex_map_path;
			}
		};

		uint32 sampler_index{m_renderCtx->getSampler(ESamplerType::eDefault)};

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
					auto albedo_map{m_renderCtx->createImageRef(*albedo_map_path)};
					material->set("albedoMap", albedo_map);
				}
				else
				{
					material->set("albedoMap", m_renderCtx->getGlobals()->debugImage());
					sampler_index = m_renderCtx->getSampler(ESamplerType::eNearest);
				}
			}
			else
			{
				material->set("albedoMap", m_renderCtx->getGlobals()->whiteImage());
			}
		}

		material->set("textureSampler", sampler_index);

		// Load normal map
		{
			// TODO: Embedded textures
			aiString ai_normal_map_path;
			if (ai_mat->GetTexture(aiTextureType_NORMALS, 0, &ai_normal_map_path) == AI_SUCCESS)
			{
				auto albedo_map_path{get_path_and_create_texture_if_exists(ai_normal_map_path, "normal")};
				if (albedo_map_path.has_value())
				{
					auto albedo_map{m_renderCtx->createImageRef(*albedo_map_path)};
					material->set("hasNormalMap", 1u);
					material->set("normalMap", albedo_map);
				}
				else
				{
					material->set("hasNormalMap", 0u);
				}
			}
			else
			{
				material->set("hasNormalMap", 0u);
			}
		}
	}

	auto DynamicMesh::getVertexBufferAddress() const -> uintptr
	{
		return vertexBufferSSBO->getDeviceAddress();
	}

	auto DynamicMesh::getIndexBufferAddress() const -> uintptr
	{
		return m_indexBuffer->getDeviceAddress();
	}

	auto DynamicMesh::getMeshData() const -> const DynamicMeshData &
	{
		return meshData;
	}

	auto DynamicMesh::getMaterials() const -> const std::vector<DynamicMaterialHandle> &
	{
		return m_materials;
	}

	auto DynamicMesh::getMaterials() -> std::vector<DynamicMaterialHandle> &
	{
		return m_materials;
	}

	auto DynamicMesh::getMaterial(uint32 p_index) const -> const DynamicMaterialHandle &
	{
		return m_materials.at(p_index);
	}

	auto DynamicMesh::getMaterial(uint32 p_index) -> DynamicMaterialHandle &
	{
		return m_materials.at(p_index);
	}

	auto DynamicMesh::getMaterialType(uint32 p_index) const -> EMaterialType
	{
		return m_materialTypes.at(p_index);
	}

	auto importMeshFromScene(const void *p_scene, DynamicMeshData &p_out_mesh_data) -> void
	{
		TST_PERMA_ASSERT_MSG(p_scene, "Scene cannot be nullptr!");
		const auto scene{static_cast<const aiScene *>(p_scene)};
		TST_PERMA_ASSERT(scene->HasMeshes());

		uint32 vertex_count{0u};
		uint32 index_count{0u};

		for (uint32 m{0u}; m < scene->mNumMeshes; ++m)
		{
			const aiMesh *ai_submesh{scene->mMeshes[m]};

			TST_PERMA_ASSERT_MSG(ai_submesh->HasPositions() && ai_submesh->HasNormals(), "What kind of mesh is ts?!");

			SubmeshData &submesh{p_out_mesh_data.submeshes.emplace_back()};
			submesh.name          = ai_submesh->mName.C_Str();
			submesh.materialIndex = ai_submesh->mMaterialIndex;
			submesh.vertexOffset  = vertex_count;
			submesh.vertexCount   = ai_submesh->mNumVertices;
			submesh.indexOffset   = index_count;
			submesh.indexCount    = ai_submesh->mNumFaces * 3u; // There are 3 indices per triangle face

			vertex_count += submesh.vertexCount;
			index_count  += submesh.indexCount;

			std::vector<DynamicMeshVertex> raw_vertices(submesh.vertexCount);
			for (uint32 i{0u}; i < submesh.vertexCount; ++i)
			{
				raw_vertices[i].position = {ai_submesh->mVertices[i].x, ai_submesh->mVertices[i].y, ai_submesh->mVertices[i].z, 1.0f};

				if (ai_submesh->HasNormals())
					raw_vertices[i].normal = {ai_submesh->mNormals[i].x, ai_submesh->mNormals[i].y, ai_submesh->mNormals[i].z, 1.0f};

				if (ai_submesh->HasTangentsAndBitangents())
				{
					raw_vertices[i].tangent   = {ai_submesh->mTangents[i].x, ai_submesh->mTangents[i].y, ai_submesh->mTangents[i].z, 1.0f};
					raw_vertices[i].bitangent = {ai_submesh->mBitangents[i].x, ai_submesh->mBitangents[i].y, ai_submesh->mBitangents[i].z, 1.0f};
				}

				if (ai_submesh->HasTextureCoords(0))
					raw_vertices[i].texCoord = {ai_submesh->mTextureCoords[0][i].x, ai_submesh->mTextureCoords[0][i].y};
				else
					raw_vertices[i].texCoord = {0.0f, 0.0f};
			}

			std::vector<uint32> raw_indices(submesh.indexCount);
			for (uint32 i{0u}; i < ai_submesh->mNumFaces; ++i)
			{
				aiFace face{ai_submesh->mFaces[i]};
				for (uint32 j{0u}; j < face.mNumIndices; ++j)
					raw_indices[(i * face.mNumIndices) + j] = face.mIndices[j];
			}

			p_out_mesh_data.vertices.insert(p_out_mesh_data.vertices.end(), raw_vertices.begin(), raw_vertices.end());
			p_out_mesh_data.indices.insert(p_out_mesh_data.indices.end(), raw_indices.begin(), raw_indices.end());
		}

		p_out_mesh_data.nodes.emplace_back();
		traverseNodes(p_out_mesh_data, scene->mRootNode, 0, Dx::XMMatrixIdentity());

		if (scene->HasAnimations())
		{
			for (uint32 i{0u}; i < scene->mNumAnimations; ++i)
			{
				aiAnimation *anim{scene->mAnimations[i]};

				aiString anim_name{anim->mName};
			}

			for (uint32 i{0u}; i < scene->mNumSkeletons; ++i)
			{
				aiSkeleton *skele{scene->mSkeletons[i]};

				aiString skele_name{skele->mName};
			}
		}
	}

	auto XM_CALLCONV traverseNodes(DynamicMeshData &p_out_mesh_data, void *p_node, uint32 p_node_index, Dx::FXMMATRIX p_parent_transform) -> void
	{
		auto ai_node{static_cast<aiNode *>(p_node)};

		Dx::XMFLOAT4X4 local_transform = mat4FromAIMatrix4x4(ai_node->mTransformation);

		Dx::XMMATRIX local_transform_simd{Dx::XMLoadFloat4x4(&local_transform)};

		Dx::XMMATRIX global_transform_simd = Dx::XMMatrixMultiply(local_transform_simd, p_parent_transform);

		Dx::XMFLOAT4X4 global_transform;
		Dx::XMStoreFloat4x4(&global_transform, global_transform_simd);

		DynamicMeshNode &node{p_out_mesh_data.nodes[p_node_index]};
		node.name      = ai_node->mName.C_Str();
		node.transform = local_transform;

		for (uint32 i{0u}; i < ai_node->mNumMeshes; ++i)
		{
			uint32 submesh_index{ai_node->mMeshes[i]};
			auto & submesh{p_out_mesh_data.submeshes[submesh_index]};
			submesh.transform      = global_transform;
			submesh.localTransform = local_transform;

			node.submeshes.emplace_back(submesh_index);
		}

		node.children.resize(ai_node->mNumChildren);

		for (uint32 i{0u}; i < ai_node->mNumChildren; ++i)
		{
			p_out_mesh_data.nodes.emplace_back();
			uint32 child_index{static_cast<uint32>(p_out_mesh_data.nodes.size()) - 1u};

			DynamicMeshNode &parent_node{p_out_mesh_data.nodes[p_node_index]};
			parent_node.children[i] = child_index;

			p_out_mesh_data.nodes[child_index].parent = p_node_index; // FIX: Use explicit p_node_index

			traverseNodes(p_out_mesh_data, ai_node->mChildren[i], child_index, global_transform_simd);
		}
	}
}
