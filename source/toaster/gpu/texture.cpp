#include "texture.hpp"
#include "gpu_context.hpp"

#include <stb/stb_image.h>

namespace toaster::gpu
{
	Texture::Texture(GPUContext *p_ctx, const std::string &p_path) : m_gpuContext(p_ctx)
	{
		#if 0
		const vk::Device device = m_gpuContext->getLogicalDevice();

		int32  channels;
		uint8 *pixels = stbi_load(p_path.c_str(), &m_width, &m_height, &channels, STBI_rgb_alpha);

		vk::DeviceSize image_size = m_width * m_height * 4;

		if (!pixels)
		{
			throw std::runtime_error("failed to load image!");
		}

		vk::Buffer       staging_buffer;
		vk::DeviceMemory staging_memory;

		m_gpuContext->createBuffer(image_size, vk::BufferUsageFlagBits::eTransferSrc,
								   vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, staging_buffer, staging_memory);

		void *data;
		vk::detail::resultCheck(device.mapMemory(staging_memory, 0, image_size, static_cast<vk::MemoryMapFlagBits>(0), &data), "Failed to map image memory");
		std::memcpy(data, pixels, image_size);
		device.unmapMemory(staging_memory);

		stbi_image_free(pixels);

		m_gpuContext->createImage(m_width, m_height, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal,
								  vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, m_image,
								  m_deviceMemory);

		m_gpuContext->transitionImageLayout(m_image, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

		m_gpuContext->copyBufferToImage(staging_buffer, m_image, m_width, m_height);

		m_gpuContext->transitionImageLayout(m_image, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
		m_imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		device.destroyBuffer(staging_buffer);
		device.freeMemory(staging_memory);

		vk::ImageViewCreateInfo viewInfo{};
		viewInfo.image                           = m_image;
		viewInfo.viewType                        = vk::ImageViewType::e2D;
		viewInfo.format                          = vk::Format::eR8G8B8A8Srgb;
		viewInfo.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor;
		viewInfo.subresourceRange.baseMipLevel   = 0;
		viewInfo.subresourceRange.levelCount     = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount     = 1;

		m_imageView = device.createImageView(viewInfo);

		vk::SamplerCreateInfo sampler_create_info{};
		sampler_create_info.magFilter               = vk::Filter::eLinear;
		sampler_create_info.minFilter               = vk::Filter::eLinear;
		sampler_create_info.addressModeU            = vk::SamplerAddressMode::eRepeat;
		sampler_create_info.addressModeV            = vk::SamplerAddressMode::eRepeat;
		sampler_create_info.addressModeW            = vk::SamplerAddressMode::eRepeat;
		sampler_create_info.anisotropyEnable        = vk::True;
		sampler_create_info.maxAnisotropy           = m_gpuContext->getPhysicalDeviceProperties().limits.maxSamplerAnisotropy;
		sampler_create_info.borderColor             = vk::BorderColor::eIntOpaqueBlack;
		sampler_create_info.unnormalizedCoordinates = vk::False;
		sampler_create_info.compareEnable           = vk::False;
		sampler_create_info.compareOp               = vk::CompareOp::eAlways;
		sampler_create_info.mipmapMode              = vk::SamplerMipmapMode::eLinear;
		sampler_create_info.mipLodBias              = 0.0f;
		sampler_create_info.minLod                  = 0.0f;
		sampler_create_info.maxLod                  = 0.0f;

		m_sampler = device.createSampler(sampler_create_info);
		#endif
	}

	Texture::~Texture()
	{
		const vk::Device device = m_gpuContext->getLogicalDevice();

		device.destroySampler(m_sampler);
		device.destroyImageView(m_imageView);
		device.destroyImage(m_image);
		device.freeMemory(m_deviceMemory);
	}

	void Texture::updateDescriptor()
	{
		m_descriptor.imageView   = m_imageView;
		m_descriptor.imageLayout = m_imageLayout;
		m_descriptor.sampler     = m_sampler;
	}

	const vk::DescriptorImageInfo *Texture::getDescriptorImageInfo() const
	{
		return &m_descriptor;
	}
}
