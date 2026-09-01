#pragma once

#include "toast_asset.hpp"
#include "toast_gpu/command_list.hpp"
#include "toast_gpu/device.hpp"

namespace toaster::asset
{
	// You will have to call DataBuffer::release() when you are done.
	auto TST_ASSET_API loadTextureIntoBuffer(CString p_path, vk::Format &p_out_format, uint32 &p_out_width, uint32 &p_out_height) -> DataBuffer;

	auto TST_ASSET_API createSampledTexture(gpu::Device &p_device, gpu::CommandList &p_cmd, CString p_filepath) -> gpu::TextureRef;
}
