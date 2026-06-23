#pragma once

#include "image.hpp"
#include "storage_buffer.hpp"
#include "toast_render.hpp"

#include "toast_gpu/vk/vk_buffer.hpp"
#include "toast_lib/io/filesystem.hpp"

namespace toaster::render
{
	struct TST_RENDER_API alignas(16) DynamicMeshVertex
	{
		Dx::XMFLOAT4 position;
		Dx::XMFLOAT3 normal;
		float32      _padd[1];
		Dx::XMFLOAT2 texCoord;
		float32      _padd2[2];
	};

	struct TST_RENDER_API alignas(16) Meshlet
	{
		uint32 vertexOffset{0u};
		uint32 triangleOffset{0u};
		uint32 vertexCount{0u};
		uint32 triangleCount{0u};

		uint32  submeshIndex{0u};
		float32 _padd[3];
	};

	struct TST_RENDER_API MeshletBounds
	{
		Dx::XMFLOAT3 center;
		float32      radius;
	};

	struct TST_RENDER_API SubmeshData
	{
		Dx::XMFLOAT4X4 modelMatrix{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
		uint32         materialIndex;
		float32        _padd[3];
	};

	struct TST_RENDER_API DynamicMaterial
	{
		ImageHandle albedoMap{nullptr};
	};

	struct TST_RENDER_API DynamicMaterialGPUData
	{
		uint32  samplerIndex;
		uint32  albedoMapIndex;
		float32 _padd[2];

		Dx::XMFLOAT4 albedoColour;
	};

	struct TST_RENDER_API DynamicMeshData
	{
		std::vector<DynamicMeshVertex> vertices;
		std::vector<Meshlet>           meshlets;
		std::vector<MeshletBounds>     meshletBounds;
		std::vector<uint32>            meshletVertices;
		std::vector<uint8>             meshletTriangles;

		std::vector<SubmeshData> submeshes;

		std::vector<DynamicMaterial>        materials;
		std::vector<DynamicMaterialGPUData> materialsGPUData;
	};

	class TST_RENDER_API DynamicMesh
	{
		TST_RENDER_OBJECT
	public:
		DynamicMesh(RenderContext &p_render_ctx, const io::filesystem::Path &p_path);

		auto getVertexBufferAddress() const -> uintptr;
		auto getMeshletBufferAddress() const -> uintptr;
		auto getMeshletVertexIndexBufferAddress() const -> uintptr;
		auto getMeshletTriangleIndexBufferAddress() const -> uintptr;
		auto getSubmeshBufferAddress() const -> uintptr;
		auto getMaterialBufferAddress() const -> uintptr;

		auto getMeshData() const -> const DynamicMeshData &;

	private:
		auto _createMaterial(void *p_mat, uint32 p_mat_index, const io::filesystem::Path &p_parent_path) -> void;

		StorageBufferUnique vertexBufferSSBO{nullptr};
		StorageBufferUnique meshletBufferSSBO{nullptr};
		StorageBufferUnique meshletVertexIndexBufferSSBO{nullptr};
		StorageBufferUnique meshletTriangleIndexBufferSSBO{nullptr};
		StorageBufferUnique submeshBufferSSBO{nullptr};
		StorageBufferUnique materialBufferSSBO{nullptr};

		DynamicMeshData meshData;
	};

	TST_RENDER_DEFINE_HANDLE(DynamicMesh, DynamicMesh);

	TST_RENDER_API auto importMeshFromScene(const void *p_scene, DynamicMeshData &p_out_mesh_data) -> void; // const aiScene*
}
