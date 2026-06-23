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

		vertexBufferSSBO               = m_renderCtx->createUnique<StorageBuffer>(meshData.vertices, "Raw_mesh_vertices");
		meshletBufferSSBO              = m_renderCtx->createUnique<StorageBuffer>(meshData.meshlets, "Meshlets");
		meshletVertexIndexBufferSSBO   = m_renderCtx->createUnique<StorageBuffer>(meshData.meshletVertices, "Meshlet_vertices");
		meshletTriangleIndexBufferSSBO = m_renderCtx->createUnique<StorageBuffer>(meshData.meshletTriangles, "Meshlet_triangles");

		submeshBufferSSBO = m_renderCtx->createUnique<StorageBuffer>(meshData.submeshes, "Submeshes");

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

		materialBufferSSBO = m_renderCtx->createUnique<StorageBuffer>(meshData.materialsGPUData, "Materials");
	}

	auto DynamicMesh::_createMaterial(void *p_mat, uint32 p_mat_index, const io::filesystem::Path &p_parent_path) -> void
	{
		auto ai_mat{static_cast<aiMaterial *>(p_mat)};

		String material_name{ai_mat->GetName().C_Str()};
		LOG_INFO("Creating new material: {}", material_name);

		auto &mat_data{meshData.materials.emplace_back()};
		auto &gpu_mat_data{meshData.materialsGPUData.emplace_back()};

		// Albedo/base colour
		{
			// if (aiColor3D ai_colour; ai_mat->Get(AI_MATKEY_COLOR_DIFFUSE, ai_colour) == AI_SUCCESS)
			// gpu_mat_data.albedoColour = {ai_colour.r, ai_colour.g, ai_colour.b, 1.0f};

			gpu_mat_data.albedoColour = {1.0f, 1.0f, 1.0f, 1.0f};
		}
		#if 0
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
		#endif

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
				gpu_mat_data.albedoMapIndex = m_renderCtx->getGlobals()->debugImage()->getAlignedShaderReadHeapID();
				gpu_mat_data.samplerIndex   = m_renderCtx->getSampler(ESamplerType::eNearest);
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

	auto DynamicMesh::getMeshletBufferAddress() const -> uintptr
	{
		return meshletBufferSSBO->getDeviceAddress();
	}

	auto DynamicMesh::getMeshletVertexIndexBufferAddress() const -> uintptr
	{
		return meshletVertexIndexBufferSSBO->getDeviceAddress();
	}

	auto DynamicMesh::getMeshletTriangleIndexBufferAddress() const -> uintptr
	{
		return meshletTriangleIndexBufferSSBO->getDeviceAddress();
	}

	auto DynamicMesh::getSubmeshBufferAddress() const -> uintptr
	{
		return submeshBufferSSBO->getDeviceAddress();
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

		uint32 current_vertex_offset{0u};
		for (uint32 m{0u}; m < scene->mNumMeshes; ++m)
		{
			const aiMesh *ai_submesh{scene->mMeshes[m]};

			std::vector<DynamicMeshVertex> raw_vertices(ai_submesh->mNumVertices);
			for (uint32 i{0u}; i < ai_submesh->mNumVertices; ++i)
			{
				raw_vertices[i].position = {ai_submesh->mVertices[i].x, ai_submesh->mVertices[i].y, ai_submesh->mVertices[i].z, 1.0f};

				if (ai_submesh->HasNormals())
					raw_vertices[i].normal = {ai_submesh->mNormals[i].x, ai_submesh->mNormals[i].y, ai_submesh->mNormals[i].z};

				if (ai_submesh->HasTextureCoords(0))
					raw_vertices[i].texCoord = {ai_submesh->mTextureCoords[0][i].x, ai_submesh->mTextureCoords[0][i].y};
				else
					raw_vertices[i].texCoord = {0.0f, 0.0f};
			}

			std::vector<uint32> raw_indices(ai_submesh->mNumFaces * 3u);
			for (uint32 i{0u}; i < ai_submesh->mNumFaces; ++i)
			{
				aiFace face{ai_submesh->mFaces[i]};
				for (uint32 j{0u}; j < face.mNumIndices; ++j)
					raw_indices.push_back(face.mIndices[j]);
			}

			constexpr uint64  max_vertices{64u};
			constexpr uint64  max_triangles{126u};
			constexpr float32 cone_weight{0.5f};

			uint64 max_meshlets{meshopt_buildMeshletsBound(raw_indices.size(), max_vertices, max_triangles)};

			std::vector<meshopt_Meshlet> local_meshlets(max_meshlets);
			std::vector<uint32>          local_meshlet_vertices(max_meshlets * max_vertices);
			std::vector<uint8>           local_meshlet_triangles(max_meshlets * max_triangles * 3);

			uint64 actual_meshlet_count{
				meshopt_buildMeshlets(local_meshlets.data(), local_meshlet_vertices.data(), local_meshlet_triangles.data(), raw_indices.data(), raw_indices.size(),
									  &raw_vertices[0].position.x, raw_vertices.size(), sizeof(DynamicMeshVertex), max_vertices, max_triangles, cone_weight)
			};

			local_meshlets.resize(actual_meshlet_count);

			uint32 global_vertex_index_offset{static_cast<uint32>(p_out_mesh_data.meshletVertices.size())};
			uint32 global_triangle_index_offset{static_cast<uint32>(p_out_mesh_data.meshletTriangles.size())};

			p_out_mesh_data.vertices.insert(p_out_mesh_data.vertices.end(), raw_vertices.begin(), raw_vertices.end());

			for (uint64 i{0u}; i < actual_meshlet_count; ++i)
			{
				const meshopt_Meshlet &src{local_meshlets[i]};

				Meshlet &dst{p_out_mesh_data.meshlets.emplace_back()};
				dst.vertexOffset   = global_vertex_index_offset + src.vertex_offset;
				dst.triangleOffset = global_triangle_index_offset + src.triangle_offset;
				dst.vertexCount    = src.vertex_count;
				dst.triangleCount  = src.triangle_count;

				dst.submeshIndex = m;
			}

			for (uint64 i{0u}; i < local_meshlet_vertices.size(); ++i)
				local_meshlet_vertices[i] += current_vertex_offset;

			if (!local_meshlets.empty())
			{
				const meshopt_Meshlet &last{local_meshlets.back()};

				// The total number of vertices used up until the last meshlet
				uint64 total_vertices_used{last.vertex_offset + last.vertex_count};
				p_out_mesh_data.meshletVertices.insert(p_out_mesh_data.meshletVertices.end(), local_meshlet_vertices.begin(),
													   local_meshlet_vertices.begin() + static_cast<int64>(total_vertices_used));

				// The total number of triangles used up until the last meshlet
				uint64 total_triangles_used{last.triangle_offset + last.triangle_count * 3};
				p_out_mesh_data.meshletTriangles.insert(p_out_mesh_data.meshletTriangles.end(), local_meshlet_triangles.begin(),
														local_meshlet_triangles.begin() + static_cast<int64>(total_triangles_used));
			}

			SubmeshData &submesh{p_out_mesh_data.submeshes.emplace_back()};
			submesh.materialIndex = ai_submesh->mMaterialIndex;

			current_vertex_offset += static_cast<uint32>(raw_vertices.size());
		}
	}
}
