#include "vk_texture.hpp"

#include <stb/stb_image.h>

#include "vk_gpu_context.hpp"

namespace toaster::gpu
{
	VKTexture2D::VKTexture2D(VKGPUContext *p_ctx, const TextureSpecInfo &p_spec_info) : m_ctx(p_ctx), m_specInfo(p_spec_info)
	{
		// The only reason to create an image without providing it with any data is to use it as an attachment...

		vk::ImageUsageFlags usage_flags{vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled};
		if (m_specInfo.sampleCount != vk::SampleCountFlagBits::e1)
			usage_flags |= vk::ImageUsageFlagBits::eTransientAttachment;

		ImageCreateInfo image_create_info{};
		image_create_info.width       = m_specInfo.width;
		image_create_info.height      = m_specInfo.height;
		image_create_info.usage       = usage_flags;
		image_create_info.mipCount    = m_mipLevels;
		image_create_info.sampleCount = m_specInfo.sampleCount;
		image_create_info.format      = m_specInfo.format;
		m_image                       = make_reference<VKImage2D>(m_ctx, image_create_info);

		m_ctx->transitionImageLayout(m_image->getImage(), m_image->getCurrentImageLayout(), vk::ImageLayout::eColorAttachmentOptimal, vk::AccessFlagBits::eNone,
									 vk::AccessFlagBits::eColorAttachmentWrite, vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eColorAttachmentOutput,
									 1, vk::ImageAspectFlagBits::eColor);
		m_image->setCurrentImageLayout(vk::ImageLayout::eColorAttachmentOptimal);

		const auto physical_device_props = m_ctx->getPhysicalDevice().getProperties();

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

		// Ts is purely aesthetic
		vk::SamplerCustomBorderColorCreateInfoEXT border_colour_create_info{};
		border_colour_create_info.customBorderColor = vk::ClearColorValue{1.0f, 0.0f, 1.0f, 1.0f};
		border_colour_create_info.format            = vk::Format::eR8G8B8A8Srgb;

		sampler_create_info.pNext = &border_colour_create_info;

		m_sampler = {m_ctx->getDevice(), sampler_create_info};

		m_descriptorImageInfo             = vk::DescriptorImageInfo{};
		m_descriptorImageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		m_descriptorImageInfo.imageView   = m_image->getImageView();
		m_descriptorImageInfo.sampler     = m_sampler;
	}

	VKTexture2D::VKTexture2D(VKGPUContext *p_ctx, const TextureSpecInfo &p_spec_info, const io::filesystem::Path &p_path) : m_ctx(p_ctx), m_specInfo(p_spec_info),
																															m_path(p_path)
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

		ImageCreateInfo image_create_info{};
		image_create_info.width       = m_specInfo.width;
		image_create_info.height      = m_specInfo.height;
		image_create_info.usage       = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
		image_create_info.mipCount    = m_mipLevels;
		image_create_info.sampleCount = m_specInfo.sampleCount;
		image_create_info.format      = vk::Format::eR8G8B8A8Srgb;
		m_image                       = make_reference<VKImage2D>(m_ctx, image_create_info);

		vk::raii::Buffer       staging_buffer{nullptr};
		vk::raii::DeviceMemory staging_buffer_memory{nullptr};

		m_ctx->createBuffer(image_size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
							staging_buffer, staging_buffer_memory);

		void *mapped = staging_buffer_memory.mapMemory(0, image_size, {});
		std::memcpy(mapped, pixels, image_size);
		staging_buffer_memory.unmapMemory();

		stbi_image_free(pixels);

		m_ctx->transitionImageLayout(m_image->getImage(), vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, vk::AccessFlagBits::eNone,
									 vk::AccessFlagBits::eTransferWrite, vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, m_mipLevels,
									 vk::ImageAspectFlagBits::eColor);

		m_image->setCurrentImageLayout(vk::ImageLayout::eTransferDstOptimal);

		m_ctx->copyBufferToImage(staging_buffer, m_image->getImage(), m_specInfo.width, m_specInfo.height);

		m_ctx->generateMipmaps(m_image->getImage(), image_create_info.format, m_specInfo.width, m_specInfo.height, m_mipLevels);

		m_image->setCurrentImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

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

		// Ts is purely aesthetic
		vk::SamplerCustomBorderColorCreateInfoEXT border_colour_create_info{};
		border_colour_create_info.customBorderColor = vk::ClearColorValue{1.0f, 0.0f, 1.0f, 1.0f};
		border_colour_create_info.format            = vk::Format::eR8G8B8A8Srgb;

		sampler_create_info.pNext = &border_colour_create_info;

		m_sampler = {m_ctx->getDevice(), sampler_create_info};

		m_descriptorImageInfo             = vk::DescriptorImageInfo{};
		m_descriptorImageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		m_descriptorImageInfo.imageView   = m_image->getImageView();
		m_descriptorImageInfo.sampler     = m_sampler;
	}

	VKTexture2D::VKTexture2D(VKGPUContext *p_ctx, const TextureSpecInfo &p_spec_info, void *p_data, uint64 p_size) : m_ctx(p_ctx), m_specInfo(p_spec_info), m_path("")
	{
		ImageCreateInfo image_create_info{};
		image_create_info.width       = m_specInfo.width;
		image_create_info.height      = m_specInfo.height;
		image_create_info.usage       = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
		image_create_info.mipCount    = m_mipLevels;
		image_create_info.sampleCount = m_specInfo.sampleCount;
		image_create_info.format      = vk::Format::eR8G8B8A8Unorm;
		m_image                       = make_reference<VKImage2D>(m_ctx, image_create_info);

		vk::raii::Buffer       staging_buffer{nullptr};
		vk::raii::DeviceMemory staging_buffer_memory{nullptr};

		vk::DeviceSize image_size{p_size}; // 1 Pixel * 1 Pixel * RGBA

		m_ctx->createBuffer(image_size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
							staging_buffer, staging_buffer_memory);

		void *mapped = staging_buffer_memory.mapMemory(0, image_size, {});
		std::memcpy(mapped, p_data, image_size);
		staging_buffer_memory.unmapMemory();

		m_ctx->transitionImageLayout(m_image->getImage(), vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, vk::AccessFlagBits::eNone,
									 vk::AccessFlagBits::eTransferWrite, vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, 1,
									 vk::ImageAspectFlagBits::eColor);

		m_image->setCurrentImageLayout(vk::ImageLayout::eTransferDstOptimal);

		m_ctx->copyBufferToImage(staging_buffer, m_image->getImage(), p_spec_info.width, p_spec_info.height);

		m_ctx->transitionImageLayout(m_image->getImage(), vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
									 vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead, vk::PipelineStageFlagBits::eTransfer,
									 vk::PipelineStageFlagBits::eFragmentShader, 1, vk::ImageAspectFlagBits::eColor);

		m_image->setCurrentImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

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
		sampler_create_info.borderColor             = vk::BorderColor::eIntOpaqueBlack;
		sampler_create_info.unnormalizedCoordinates = false;

		// Ts is purely aesthetic
		vk::SamplerCustomBorderColorCreateInfoEXT border_colour_create_info{};
		border_colour_create_info.customBorderColor = vk::ClearColorValue{1.0f, 0.0f, 1.0f, 1.0f};
		border_colour_create_info.format            = vk::Format::eR8G8B8A8Srgb;

		sampler_create_info.pNext = &border_colour_create_info;

		m_sampler = {m_ctx->getDevice(), sampler_create_info};

		m_descriptorImageInfo             = vk::DescriptorImageInfo{};
		m_descriptorImageInfo.imageLayout = m_image->getCurrentImageLayout();
		m_descriptorImageInfo.imageView   = m_image->getImageView();
		m_descriptorImageInfo.sampler     = m_sampler;
	}

	VKGPUContext *VKTexture2D::getContext() const
	{
		return m_ctx;
	}

	void VKTexture2D::resize(uint32 p_width, uint32 p_height)
	{
		m_sampler             = nullptr;
		m_descriptorImageInfo = vk::DescriptorImageInfo{};

		m_image->resize(p_width, p_height);

		m_image->setCurrentImageLayout(vk::ImageLayout::eColorAttachmentOptimal);

		const auto physical_device_props = m_ctx->getPhysicalDevice().getProperties();

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

		// Ts is purely aesthetic
		vk::SamplerCustomBorderColorCreateInfoEXT border_colour_create_info{};
		border_colour_create_info.customBorderColor = vk::ClearColorValue{1.0f, 0.0f, 1.0f, 1.0f};
		border_colour_create_info.format            = vk::Format::eR8G8B8A8Srgb;

		sampler_create_info.pNext = &border_colour_create_info;

		m_sampler = {m_ctx->getDevice(), sampler_create_info};

		m_descriptorImageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		m_descriptorImageInfo.imageView   = m_image->getImageView();
		m_descriptorImageInfo.sampler     = m_sampler;
	}

	const TextureSpecInfo &VKTexture2D::getSpecInfo() const
	{
		return m_specInfo;
	}

	const io::filesystem::Path &VKTexture2D::getPath() const
	{
		return m_path;
	}

	uint32 VKTexture2D::getMipLevelCount() const
	{
		return m_mipLevels;
	}

	const RefPtr<VKImage2D> &VKTexture2D::getImage() const
	{
		return m_image;
	}

	vk::raii::Sampler &VKTexture2D::getSampler()
	{
		return m_sampler;
	}

	vk::DescriptorImageInfo &VKTexture2D::getDescriptorInfo()
	{
		return m_descriptorImageInfo;
	}

	EGPUResourceType VKTexture2D::getResourceType() const
	{
		return EGPUResourceType::eTexture2D;
	}
}
