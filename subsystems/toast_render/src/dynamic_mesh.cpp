#include "toast_render/dynamic_mesh.hpp"

#include "assimp_common.hpp"
#include "meshoptimizer.h"

namespace toaster::render
{
	auto importMeshFromFile(const io::filesystem::Path &p_path) -> DynamicMeshData
	{
		Assimp::Importer importer{};

		const aiScene *scene{importer.ReadFile(p_path.string().c_str(), s_MeshImportFlags)};

		if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
		{
			LOG_ERROR("Failed to import mesh from file: {}", p_path);
			return {};
		}

		DynamicMeshData out_mesh_data{};

		uint32 current_vertex_offset{0u};
		for (uint32 m{0u}; m < scene->mNumMeshes; ++m)
		{
			const aiMesh *submesh{scene->mMeshes[m]};

			std::vector<DynamicMeshVertex> raw_vertices(submesh->mNumVertices);
			for (uint32 i{0u}; i < submesh->mNumVertices; ++i)
			{
				raw_vertices[i].position = {submesh->mVertices[i].x, submesh->mVertices[i].y, submesh->mVertices[i].z, 1.0f};

				if (submesh->HasTextureCoords(0))
					raw_vertices[i].texCoord = {submesh->mTextureCoords[0][i].x, submesh->mTextureCoords[0][i].y};
				else
					raw_vertices[i].texCoord = {0.0f, 0.0f};
			}

			std::vector<uint32> raw_indices(submesh->mNumFaces * 3u);
			for (uint32 i{0u}; i < submesh->mNumFaces; ++i)
			{
				aiFace face{submesh->mFaces[i]};
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

			uint32 global_vertex_index_offset{static_cast<uint32>(out_mesh_data.meshletVertices.size())};
			uint32 global_triangle_index_offset{static_cast<uint32>(out_mesh_data.meshletTriangles.size())};

			out_mesh_data.vertices.insert(out_mesh_data.vertices.end(), raw_vertices.begin(), raw_vertices.end());

			for (uint64 i{0u}; i < actual_meshlet_count; ++i)
			{
				const meshopt_Meshlet &src{local_meshlets[i]};

				Meshlet &dst{out_mesh_data.meshlets.emplace_back()};
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
				out_mesh_data.meshletVertices.insert(out_mesh_data.meshletVertices.end(), local_meshlet_vertices.begin(),
													 local_meshlet_vertices.begin() + static_cast<int64>(total_vertices_used));

				// The total number of triangles used up until the last meshlet
				uint64 total_triangles_used{last.triangle_offset + last.triangle_count * 3};
				out_mesh_data.meshletTriangles.insert(out_mesh_data.meshletTriangles.end(), local_meshlet_triangles.begin(),
													  local_meshlet_triangles.begin() + static_cast<int64>(total_triangles_used));
			}

			current_vertex_offset += static_cast<uint32>(raw_vertices.size());
		}

		#if 0
		aiMesh *ai_mesh{scene->mMeshes[0]}; std::vector<DynamicMeshVertex> raw_vertices(ai_mesh->mNumVertices); std::vector<uint32> raw_indices; for (
			uint32 i{0u}; i < ai_mesh->mNumVertices; ++i)
		{
			raw_vertices[i].position = {ai_mesh->mVertices[i].x, ai_mesh->mVertices[i].y, ai_mesh->mVertices[i].z, 1.0f};

			if (ai_mesh->HasTextureCoords(0))
				raw_vertices[i].texCoord = {ai_mesh->mTextureCoords[0][i].x, ai_mesh->mTextureCoords[0][i].y};
			else
				raw_vertices[i].texCoord = {0.0f, 0.0f};
		} raw_indices.reserve(ai_mesh->mNumFaces * 3u); for (uint32 i{0u}; i < ai_mesh->mNumFaces; ++i)
		{
			aiFace face{ai_mesh->mFaces[i]};
			for (uint32 j{0u}; j < face.mNumIndices; ++j)
				raw_indices.push_back(face.mIndices[j]);
		} DynamicMeshData out_mesh_data{}; out_mesh_data.vertices = raw_vertices; constexpr uint64 max_vertices{64u}; constexpr uint64 max_triangles{126u}; constexpr
				float32 cone_weight{0.5f}; uint64 max_meshlets{meshopt_buildMeshletsBound(raw_indices.size(), max_vertices, max_triangles)}; std::vector<meshopt_Meshlet>
				local_meshlets(max_meshlets); out_mesh_data.meshletVertices.resize(raw_indices.size()); out_mesh_data.meshletTriangles.resize(raw_indices.size()); uint64
				actual_meshlet_count{
					meshopt_buildMeshlets(local_meshlets.data(), out_mesh_data.meshletVertices.data(), out_mesh_data.meshletTriangles.data(), raw_indices.data(),
										  raw_indices.size(), &out_mesh_data.vertices[0].position.x, out_mesh_data.vertices.size(), sizeof(DynamicMeshVertex),
										  max_vertices, max_triangles, cone_weight)
				}; local_meshlets.resize(actual_meshlet_count); LOG_INFO("Meshlet count: {}", actual_meshlet_count); if (!local_meshlets.empty())
		{
			const auto &last_meshlet{local_meshlets.back()};
			out_mesh_data.meshletVertices.resize(last_meshlet.vertex_offset + last_meshlet.vertex_count);
			out_mesh_data.meshletTriangles.resize(last_meshlet.triangle_offset + last_meshlet.triangle_count * 3);
		} out_mesh_data.meshlets.resize(actual_meshlet_count); out_mesh_data.meshletBounds.resize(actual_meshlet_count); for (uint64 i{0u}; i < actual_meshlet_count; ++i)
		{
			out_mesh_data.meshlets[i].vertexOffset   = local_meshlets[i].vertex_offset;
			out_mesh_data.meshlets[i].triangleOffset = local_meshlets[i].triangle_offset;
			out_mesh_data.meshlets[i].vertexCount    = local_meshlets[i].vertex_count;
			out_mesh_data.meshlets[i].triangleCount  = local_meshlets[i].triangle_count;
		}

		#endif

		return out_mesh_data;
	}
}
