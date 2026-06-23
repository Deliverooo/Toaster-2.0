#pragma once

#include "toast_asset.hpp"
#include "toast_gpu/vk/vk_raw_image.hpp"

namespace toaster::asset
{
	#pragma pack(push, 1)
	struct TST_ASSET_API TextureHeader
	{
		char8  magic[4]{'T', 'T', 'E', 'X'};
		uint32 version{0x00000000u};

		uint32 width{0u};
		uint32 height{0u};
		uint32 mipLevels{0u};
		uint32 vulkanFormat{0u};
		uint64 dataSize{0u};
	};
	#pragma pack(pop)

	TST_ASSET_API auto serializeTextureToFile(gpu::VKLogicalDevice &p_device, gpu::RawImage &p_image, const io::filesystem::Path &p_dst_path) -> void;
}
