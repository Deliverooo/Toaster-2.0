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

	auto createSampledTexture(gpu::ResourceManager &p_resource_manager,gpu::CommandList &p_cmd, CString p_filepath) -> gpu::TextureHandle
	{
		gpu::TextureDesc texture_desc{};
		texture_desc.layerCount        = 1u;
		texture_desc.mipLevels         = 1u;
		texture_desc.type              = vk::ImageType::e2D;
		texture_desc.usageFlags        = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
		texture_desc.createDescriptors = true;
		texture_desc.extent.depth      = 1u;
		DataBuffer texture_data{loadTextureIntoBuffer(p_filepath, texture_desc.format, texture_desc.extent.width, texture_desc.extent.height)};

		gpu::TextureHandle tex{p_resource_manager.createTexture(texture_desc)};
		p_cmd.transitionTextureLayout(p_resource_manager, tex, vk::ImageLayout::eTransferDstOptimal);

		gpu::BufferHandle buff{p_resource_manager.createBuffer(gpu::BufferDesc::staging(texture_data.size()))};
		p_resource_manager.uploadBufferData(buff, texture_data.data(), texture_data.size());
		p_cmd.copyBufferToTexture(p_resource_manager, buff, tex);
		p_resource_manager.destroyBuffer(buff);
		texture_data.release();

		p_cmd.transitionTextureLayout(p_resource_manager, tex, vk::ImageLayout::eShaderReadOnlyOptimal);

		return tex;
	}
}
