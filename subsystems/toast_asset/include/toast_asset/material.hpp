#pragma once

#include "toast_asset.hpp"
#include "texture.hpp"

namespace toaster::asset
{
	struct TST_ASSET_API MaterialAssetData
	{
		URI                     uri{};;
		static constexpr uint32 maxTextureRefs{4u};

		std::vector<uint8> data;

		// TODO: VFS/URI For memory only textures
		std::array<TextureAssetHandle, maxTextureRefs> textureRefs;

		VmaVirtualAllocation virtualAllocation{nullptr};
		uint64               allocationOffset{0u};
		uint64               allocationSize{0u};
	};

	TST_DECLARE_HANDLE(MaterialAsset);
}
