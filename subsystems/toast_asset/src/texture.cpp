#include "toast_asset/texture.hpp"

#include <stb/stb_image.h>

namespace toaster::asset
{
	auto loadTextureIntoBuffer(CString p_path, vk::Format &p_out_format, uint32 &p_out_width, uint32 &p_out_height) -> DataBuffer
	{
		DataBuffer image_data{};

		const bool is_srgb = (p_out_format == vk::Format::eR8G8B8Srgb) || (p_out_format == vk::Format::eR8G8B8A8Srgb);
		int32      width{0u};
		int32      height{0u};
		int32      num_channels{0u};

		if (stbi_is_hdr(p_path))
		{
			const auto data{reinterpret_cast<uint8 *>(stbi_loadf(p_path, &width, &height, &num_channels, 4))};
			if (!data)
				return DataBuffer{};
			TST_ASSERT_MSG(width != 0 && height != 0, "Bradar, wat is dis?");
			uint64 size{width * height * 4 * sizeof(float32)};
			image_data.allocate(size);
			image_data.write(data, size);

			p_out_format = vk::Format::eR32G32B32A32Sfloat;

			stbi_image_free(data);
		}
		else
		{
			uint8 *data{stbi_load(p_path, &width, &height, &num_channels, 4)};
			if (!data)
				return DataBuffer{};

			TST_ASSERT_MSG(width != 0 && height != 0, "Bradar, wat is dis?");
			const uint64 size{width * height * sizeof(uint32)};
			image_data.allocate(size);
			image_data.write(data, size);

			p_out_format = is_srgb ? vk::Format::eR8G8B8A8Srgb : vk::Format::eR8G8B8A8Unorm;

			stbi_image_free(data);
		}

		if (!image_data.data())
		{
			TST_ASSERT_MSG(false, "Failed to load texture");
			return DataBuffer{};
		}

		p_out_width  = width;
		p_out_height = height;
		return image_data;
	}

	auto createSampledTexture(gpu::Device &p_device, gpu::CommandList &p_cmd, CString p_filepath) -> gpu::TextureRef
	{
		gpu::TextureDesc texture_desc{};
		texture_desc.layerCount        = 1u;
		texture_desc.mipLevels         = 1u;
		texture_desc.type              = vk::ImageType::e2D;
		texture_desc.usageFlags        = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
		texture_desc.createDescriptors = true;
		texture_desc.extent.depth      = 1u;
		DataBuffer texture_data{loadTextureIntoBuffer(p_filepath, texture_desc.format, texture_desc.extent.width, texture_desc.extent.height)};

		auto tex{p_device.createTexture(texture_desc)};
		p_cmd.transitionTextureLayout(tex.get(), vk::ImageLayout::eTransferDstOptimal);

		auto buff{p_device.createBuffer(gpu::BufferDesc::staging(texture_data.size()))};
		p_device.uploadBufferData(buff.get(), texture_data.data(), texture_data.size());
		p_cmd.copyBufferToTexture(buff.get(), tex.get());
		// p_device.releaseBuffer(buff.get());
		texture_data.release();

		p_cmd.transitionTextureLayout(tex.get(), vk::ImageLayout::eShaderReadOnlyOptimal);

		return tex;
	}
}
