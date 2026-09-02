#pragma once

#include "toast_asset.hpp"
#include "material.hpp"

namespace toaster::asset
{
	struct TST_ASSET_API StaticMeshVertex
	{
		XMFLOAT3 position;
		XMFLOAT3 normal;
		XMFLOAT2 texCoord;
	};

	struct TST_ASSET_API SubmeshData
	{
		uint32 indexOffset{0u};
		int32  vertexOffset{0}; // Apparently, Vulkan wants the vertex offset as an int and not a uint.
		uint32 indexCount{0u};

		MaterialAssetHandle material{};
	};

	struct TST_ASSET_API StaticMeshAssetData
	{
		URI uri{};

		VmaVirtualAllocation vertexBufferAllocation{nullptr};
		VmaVirtualAllocation indexBufferAllocation{nullptr};

		// These are BYTE offsets... :(
		uint64 vertexBufferByteOffset{0u};
		uint64 indexBufferByteOffset{0u};

		auto getVertexBufferOffset() const -> uint32 { return static_cast<uint32>(vertexBufferByteOffset) / sizeof(StaticMeshVertex); }
		auto getIndexBufferOffset() const -> uint32 { return static_cast<uint32>(indexBufferByteOffset) / sizeof(uint32); }

		std::vector<SubmeshData> submeshes;
	};

	TST_DECLARE_HANDLE(StaticMeshAsset);
}
