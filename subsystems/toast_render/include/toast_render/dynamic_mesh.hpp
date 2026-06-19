#pragma once

#include "toast_render.hpp"

#include "toast_gpu/vk/vk_buffer.hpp"
#include "toast_lib/io/filesystem.hpp"

namespace toaster::render
{
	struct TST_RENDER_API alignas(16) DynamicMeshVertex
	{
		Dx::XMFLOAT4 position;
		// float32 _padd;
		// Dx::XMFLOAT3 normal;
		// Dx::XMFLOAT3 tangent;
		// Dx::XMFLOAT3 bitangent;
		// Dx::XMFLOAT2 texCoord;
		// float32      _padd4[3];
	};

	struct TST_RENDER_API alignas(16) Meshlet
	{
		uint32 vertexOffset{0u};
		uint32 triangleOffset{0u};
		uint32 vertexCount{0u};
		uint32 triangleCount{0u};
	};

	struct TST_RENDER_API MeshletBounds
	{
		Dx::XMFLOAT3 center;
		float32      radius;
	};

	struct TST_RENDER_API DynamicMeshData
	{
		std::vector<DynamicMeshVertex> vertices;
		std::vector<Meshlet>           meshlets;
		std::vector<MeshletBounds>     meshletBounds;
		std::vector<uint32>            meshletVertices;
		std::vector<uint32>            meshletTriangles; // Packed 4 uint8 triangle indices per uint32
	};

	TST_RENDER_API auto importMeshFromFile(const io::filesystem::Path &p_path) -> DynamicMeshData;
}
