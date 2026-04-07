#include "vk_texture.hpp"

#include <stb/stb_image.h>

#include "vk_gpu_context.hpp"

namespace toaster::gpu
{
	VKTexture2D::VKTexture2D(VKGPUContext *p_ctx, const TextureSpecInfo &p_spec_info, const io::filesystem::Path &p_path) : m_ctx(p_ctx), m_specInfo(p_spec_info)
	{
		TST_ASSERT_MSG(p_ctx, "Context cannot be null");

		// Ts is somewhat necessary
		stbi_set_flip_vertically_on_load(true);

		// Possibly in the future I might look into dynamic colour channels, so the images don't need to be in RGBA
		int32 width{0};
		int32 height{0};
		int32 num_channels{0};
		auto  pixels = stbi_load(p_path.string().c_str(), &width, &height, &num_channels, 4);

		if (!pixels)
			TST_ASSERT_MSG(false, "failed to load texture image");

		vk::DeviceSize image_size = width * height * 4;
		m_specInfo.width          = static_cast<uint32>(width);
		m_specInfo.height         = static_cast<uint32>(height);

		if (m_specInfo.generateMips)
			m_mipLevels = std::floor(std::log2(std::max(m_specInfo.width, m_specInfo.height))) + 1u;
		else
			m_mipLevels = 1;

		vk::raii::Buffer       staging_buffer{nullptr};
		vk::raii::DeviceMemory staging_buffer_memory{nullptr};

		m_ctx->createBuffer(image_size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
							staging_buffer, staging_buffer_memory);

		void *mapped = staging_buffer_memory.mapMemory(0, image_size, {});
		std::memcpy(mapped, pixels, image_size);
		staging_buffer_memory.unmapMemory();

		stbi_image_free(pixels);

		vk::Format image_format = vk::Format::eR8G8B8A8Srgb;
		m_ctx->createImage(m_specInfo.width, m_specInfo.height, m_mipLevels, image_format, vk::ImageTiling::eOptimal,
						   vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
						   vk::MemoryPropertyFlagBits::eDeviceLocal, m_image, m_imageMemory);

		m_ctx->transitionImageLayout(m_image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, vk::AccessFlagBits::eNone,
									 vk::AccessFlagBits::eTransferWrite, vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, m_mipLevels);

		m_ctx->copyBufferToImage(staging_buffer, m_image, m_specInfo.width, m_specInfo.height);

		m_ctx->generateMipmaps(m_image, image_format, m_specInfo.width, m_specInfo.height, m_mipLevels);

		m_imageView = m_ctx->createImageView(m_image, image_format, vk::ImageAspectFlagBits::eColor, m_mipLevels);

		auto physical_device_props = m_ctx->getPhysicalDevice().getProperties();

		vk::SamplerCreateInfo sampler_create_info{};
		sampler_create_info.addressModeU            = vk::SamplerAddressMode::eRepeat;
		sampler_create_info.addressModeV            = vk::SamplerAddressMode::eRepeat;
		sampler_create_info.addressModeW            = vk::SamplerAddressMode::eRepeat;
		sampler_create_info.magFilter               = vk::Filter::eLinear;
		sampler_create_info.minFilter               = vk::Filter::eLinear;
		sampler_create_info.mipmapMode              = vk::SamplerMipmapMode::eLinear;
		sampler_create_info.addressModeU            = vk::SamplerAddressMode::eRepeat;
		sampler_create_info.addressModeV            = vk::SamplerAddressMode::eRepeat;
		sampler_create_info.addressModeW            = vk::SamplerAddressMode::eRepeat;
		sampler_create_info.mipLodBias              = 0.0f;
		sampler_create_info.anisotropyEnable        = true;
		sampler_create_info.maxAnisotropy           = physical_device_props.limits.maxSamplerAnisotropy;
		sampler_create_info.compareEnable           = false;
		sampler_create_info.compareOp               = vk::CompareOp::eAlways;
		sampler_create_info.minLod                  = 0.0f;
		sampler_create_info.maxLod                  = vk::LodClampNone;
		sampler_create_info.borderColor             = vk::BorderColor::eFloatCustomEXT;
		sampler_create_info.unnormalizedCoordinates = false;

		// This is purely aesthetic
		vk::SamplerCustomBorderColorCreateInfoEXT border_colour_create_info{};
		border_colour_create_info.customBorderColor = vk::ClearColorValue{1.0f, 0.0f, 1.0f, 1.0f};
		border_colour_create_info.format            = vk::Format::eR8G8B8A8Srgb;

		sampler_create_info.pNext = &border_colour_create_info;

		m_sampler = {m_ctx->getDevice(), sampler_create_info};

		m_descriptorImageInfo             = vk::DescriptorImageInfo{};
		m_descriptorImageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		m_descriptorImageInfo.imageView   = m_imageView;
		m_descriptorImageInfo.sampler     = m_sampler;
	}

	vk::raii::Image &VKTexture2D::getImage()
	{
		return m_image;
	}

	vk::raii::DeviceMemory &VKTexture2D::getImageMemory()
	{
		return m_imageMemory;
	}

	vk::raii::ImageView &VKTexture2D::getImageView()
	{
		return m_imageView;
	}

	vk::raii::Sampler &VKTexture2D::getSampler()
	{
		return m_sampler;
	}

	vk::DescriptorImageInfo &VKTexture2D::getDescriptorInfo()
	{
		return m_descriptorImageInfo;
	}

	const TextureSpecInfo &VKTexture2D::getSpecInfo() const
	{
		return m_specInfo;
	}

	EResourceType VKTexture2D::getResourceType() const
	{
		return EResourceType::eTexture2D;
	}
}
