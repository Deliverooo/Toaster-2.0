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

		aiMesh *ai_mesh{scene->mMeshes[0]};

		std::vector<DynamicMeshVertex> raw_vertices(ai_mesh->mNumVertices);
		std::vector<uint32>            raw_indices;

		for (uint32 i{0u}; i < ai_mesh->mNumVertices; ++i)
		{
			raw_vertices[i].position = {ai_mesh->mVertices[i].x, ai_mesh->mVertices[i].y, ai_mesh->mVertices[i].z, 1.0f};

			// if (ai_mesh->HasNormals())
				// raw_vertices[i].normal = {ai_mesh->mNormals[i].x, ai_mesh->mNormals[i].y, ai_mesh->mNormals[i].z};

			// if (ai_mesh->HasTangentsAndBitangents())
			// {
				// raw_vertices[i].tangent   = {ai_mesh->mTangents[i].x, ai_mesh->mTangents[i].y, ai_mesh->mTangents[i].z};
				// raw_vertices[i].bitangent = {ai_mesh->mBitangents[i].x, ai_mesh->mBitangents[i].y, ai_mesh->mBitangents[i].z};
			// }

			// if (ai_mesh->HasTextureCoords(0))
				// raw_vertices[i].texCoord = {ai_mesh->mTextureCoords[0][i].x, ai_mesh->mTextureCoords[0][i].y};
			// else
				// raw_vertices[i].texCoord = {0.0f, 0.0f};
		}

		raw_indices.reserve(ai_mesh->mNumFaces * 3u);
		for (uint32 i{0u}; i < ai_mesh->mNumFaces; ++i)
		{
			aiFace face{ai_mesh->mFaces[i]};
			for (uint32 j{0u}; j < face.mNumIndices; ++j)
				raw_indices.push_back(face.mIndices[j]);
		}

		DynamicMeshData out_mesh_data{};
		out_mesh_data.vertices = raw_vertices;

		constexpr uint64  max_vertices{64u};
		constexpr uint64  max_triangles{126u};
		constexpr float32 cone_weight{0.5f};

		uint64 max_meshlets{meshopt_buildMeshletsBound(raw_indices.size(), max_vertices, max_triangles)};

		std::vector<meshopt_Meshlet> local_meshlets(max_meshlets);
		out_mesh_data.meshletVertices.resize(raw_indices.size());
		std::vector<uint8> raw_triangle_data(raw_indices.size());

		uint64 actual_meshlet_count{
			meshopt_buildMeshlets(local_meshlets.data(), out_mesh_data.meshletVertices.data(), raw_triangle_data.data(), raw_indices.data(),
								  raw_indices.size(), &out_mesh_data.vertices[0].position.x, out_mesh_data.vertices.size(), sizeof(DynamicMeshVertex), max_vertices,
								  max_triangles, cone_weight)
		};

		local_meshlets.resize(actual_meshlet_count);

		LOG_INFO("Meshlet count: {}", actual_meshlet_count);

		if (!local_meshlets.empty())
		{
			const auto &last_meshlet{local_meshlets.back()};
			out_mesh_data.meshletVertices.resize(last_meshlet.vertex_offset + last_meshlet.vertex_count);
			raw_triangle_data.resize(last_meshlet.triangle_offset + last_meshlet.triangle_count * 3);
		}

		// Pack uint8 triangle indices into uint32 for proper alignment in shader
		out_mesh_data.meshletTriangles.resize((raw_triangle_data.size() + 3) / 4);
		for (uint64 i{0u}; i < raw_triangle_data.size(); ++i)
		{
			uint32 &packed = out_mesh_data.meshletTriangles[i / 4];
			packed |= (uint32)raw_triangle_data[i] << ((i % 4) * 8);
		}

		out_mesh_data.meshlets.resize(actual_meshlet_count);
		out_mesh_data.meshletBounds.resize(actual_meshlet_count);

		for (uint64 i{0u}; i < actual_meshlet_count; ++i)
		{
			out_mesh_data.meshlets[i].vertexOffset   = local_meshlets[i].vertex_offset;
			out_mesh_data.meshlets[i].triangleOffset = local_meshlets[i].triangle_offset;
			out_mesh_data.meshlets[i].vertexCount    = local_meshlets[i].vertex_count;
			out_mesh_data.meshlets[i].triangleCount  = local_meshlets[i].triangle_count;
		}

		return out_mesh_data;
	}
}
