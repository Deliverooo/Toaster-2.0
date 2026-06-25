#include "toast_render/dynamic_mesh.hpp"

#include "assimp_common.hpp"
#include "meshoptimizer.h"
#include "toast_render/globals.hpp"

namespace toaster::render
{
	DynamicMesh::DynamicMesh(RenderContext &p_render_ctx, const io::filesystem::Path &p_path) : m_renderCtx(&p_render_ctx)
	{
		Assimp::Importer importer;

		const aiScene *scene = importer.ReadFile(p_path.string(), s_MeshImportFlags);
		if (!scene)
		{
			LOG_ERROR("Failed to load mesh file: {0}", p_path.string());
			TST_PERMA_ASSERT(false);
		}
		importMeshFromScene(scene, meshData);

		uint64              index_buffer_size{meshData.indices.size() * sizeof(uint32)};
		gpu::BufferSpecInfo staging_buffer_spec{};
		staging_buffer_spec.deviceLocal = false;
		staging_buffer_spec.usageFlags  = vk::BufferUsageFlagBits2::eTransferSrc;
		gpu::Buffer staging_buffer{m_renderCtx->getLogicalDevice(), index_buffer_size, staging_buffer_spec};
		staging_buffer.setData(meshData.indices.data(), index_buffer_size);

		gpu::BufferSpecInfo index_buffer_spec_info;
		index_buffer_spec_info.deviceLocal = true;
		index_buffer_spec_info.usageFlags  = vk::BufferUsageFlagBits2::eTransferDst | vk::BufferUsageFlagBits2::eIndexBuffer;
		m_indexBuffer                      = m_renderCtx->createGPUUnique<gpu::Buffer>(index_buffer_size, index_buffer_spec_info);

		m_indexBuffer->copyFromBuffer(staging_buffer);

		vertexBufferSSBO = m_renderCtx->createUnique<StorageBuffer>(meshData.vertices, true);

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

		materialBufferSSBO = m_renderCtx->createUnique<StorageBuffer>(meshData.materialsGPUData, false);
	}

	auto DynamicMesh::getIndexBuffer() const -> const gpu::Buffer &
	{
		return *m_indexBuffer;
	}

	auto DynamicMesh::_createMaterial(void *p_mat, uint32 p_mat_index, const io::filesystem::Path &p_parent_path) -> void
	{
		auto ai_mat{static_cast<aiMaterial *>(p_mat)};

		String material_name{ai_mat->GetName().C_Str()};
		LOG_INFO("Creating new material: {}", material_name);

		auto &mat_data{meshData.materials.emplace_back()};
		auto &gpu_mat_data{meshData.materialsGPUData.emplace_back()};

		if (aiColor3D ai_colour{1.0f, 1.0f, 1.0f}; ai_mat->Get(AI_MATKEY_COLOR_DIFFUSE, ai_colour) == AI_SUCCESS)
			gpu_mat_data.albedoColour = {ai_colour.r, ai_colour.g, ai_colour.b, 1.0f};

		if (float32 roughness{0.4f}; ai_mat->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == AI_SUCCESS)
			gpu_mat_data.roughness = roughness;

		if (float32 metalness{0.0f}; ai_mat->Get(AI_MATKEY_REFLECTIVITY, metalness) == AI_SUCCESS)
			gpu_mat_data.metalness = metalness;

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

		gpu_mat_data.samplerIndex = m_renderCtx->getSampler(ESamplerType::eDefault);

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
					mat_data.albedoMap          = m_renderCtx->createImageRef(*albedo_map_path);
					gpu_mat_data.albedoMapIndex = mat_data.albedoMap->getAlignedShaderReadHeapID();
				}
				else
				{
					gpu_mat_data.albedoMapIndex = m_renderCtx->getGlobals()->debugImage()->getAlignedShaderReadHeapID();
					gpu_mat_data.samplerIndex   = m_renderCtx->getSampler(ESamplerType::eNearest);
				}
			}
			else
			{
				gpu_mat_data.albedoMapIndex = m_renderCtx->getGlobals()->whiteImage()->getAlignedShaderReadHeapID();
				LOG_WARN("\tMaterial '{}' does not have an albedo map", material_name);
			}
		}
		#if 0

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
					mat_data.setNormalMap(m_renderCtx->createGPURef<gpu::VKTexture2D>(texture_spec_info, *normal_map_path));
				}
				material->set<uint32>("u_Material.hasNormalMap", 1u);
			}
			else
			{
				material->set<uint32>("u_Material.hasNormalMap", 0u);
				LOG_WARN("\tMaterial '{}' does not have a normal map", material_name);
			}
		}

		#endif
	}

	auto DynamicMesh::getVertexBufferAddress() const -> uintptr
	{
		return vertexBufferSSBO->getDeviceAddress();
	}

	auto DynamicMesh::getIndexBufferAddress() const -> uintptr
	{
		return m_indexBuffer->getDeviceAddress();
	}

	auto DynamicMesh::getMaterialBufferAddress() const -> uintptr
	{
		return materialBufferSSBO->getDeviceAddress();
	}

	auto DynamicMesh::getMeshData() const -> const DynamicMeshData &
	{
		return meshData;
	}

	auto importMeshFromScene(const void *p_scene, DynamicMeshData &p_out_mesh_data) -> void
	{
		TST_PERMA_ASSERT_MSG(p_scene, "Scene cannot be nullptr!");
		const auto scene{static_cast<const aiScene *>(p_scene)};
		TST_PERMA_ASSERT(scene->HasMeshes());

		uint32 vertex_count{0u};
		uint32 index_count{0u};

		// uint32 current_vertex_offset{0u};
		for (uint32 m{0u}; m < scene->mNumMeshes; ++m)
		{
			const aiMesh *ai_submesh{scene->mMeshes[m]};

			TST_PERMA_ASSERT_MSG(ai_submesh->HasPositions() && ai_submesh->HasNormals(), "What kind of mesh is ts?!");

			SubmeshData &submesh{p_out_mesh_data.submeshes.emplace_back()};
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
					raw_vertices[i].normal = {ai_submesh->mNormals[i].x, ai_submesh->mNormals[i].y, ai_submesh->mNormals[i].z};

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
	}
}
