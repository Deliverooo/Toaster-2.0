#include "toast_gpu/vk/vk_texture.hpp"
#include "toast_gpu/vk/vk_logical_device.hpp"

#include <stb/stb_image.h>

namespace toaster::gpu
{
	VKTexture2D::VKTexture2D(VKLogicalDevice *p_device, const TextureSpecInfo &p_spec_info) : m_device(p_device), m_specInfo(p_spec_info)
	{
		TST_ASSERT_MSG(m_device, "Device cannot be null");

		vk::ImageUsageFlags usage_flags{vk::ImageUsageFlagBits::eSampled};
		usage_flags |= util::getImageUsageFlags(m_specInfo.format, m_specInfo.sampleCount);

		// if (m_specInfo.usage == ETextureUsage::eShaderSampled)
		usage_flags |= vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst;

		ImageSpecInfo image_create_info{};
		image_create_info.size        = m_specInfo.size;
		image_create_info.usage       = usage_flags;
		image_create_info.mipCount    = m_mipLevels;
		image_create_info.sampleCount = m_specInfo.sampleCount;
		image_create_info.format      = m_specInfo.format;
		m_image                       = make_reference<VKRawImage>(m_device, image_create_info);

		createSampler(vk::ImageLayout::eShaderReadOnlyOptimal);
	}

	VKTexture2D::VKTexture2D(VKLogicalDevice *p_device, const TextureSpecInfo &p_spec_info, const io::filesystem::Path &p_path) : m_device(p_device),
																																  m_specInfo(p_spec_info), m_path(p_path)
	{
		TST_ASSERT_MSG(m_device, "Device cannot be null");

		m_textureData = util::loadTextureIntoBuffer(p_path, m_specInfo.format, m_specInfo.size.x, m_specInfo.size.y);
		if (!m_textureData)
		{
			TST_ASSERT(false);
		}

		ImageSpecInfo image_create_info{};
		image_create_info.size        = m_specInfo.size;
		image_create_info.usage       = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
		image_create_info.mipCount    = m_mipLevels;
		image_create_info.sampleCount = m_specInfo.sampleCount;
		image_create_info.format      = m_specInfo.format;
		m_image                       = make_reference<VKRawImage>(m_device, image_create_info);

		util::toTransferDst(m_image.get());
		m_image->setData(m_textureData);

		m_device->generateMipmaps(m_image->getImage(), {m_specInfo.size.x, m_specInfo.size.y, 1u}, m_mipLevels);
		m_image->setCurrentImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal); // Generate mips leaves the image in the eShaderReadOnlyOptimal layout

		createSampler();
	}

	VKTexture2D::VKTexture2D(VKLogicalDevice *p_device, const TextureSpecInfo &p_spec_info, void *p_data, uint64 p_size) : m_device(p_device), m_specInfo(p_spec_info),
																														   m_path("")
	{
		TST_ASSERT_MSG(m_device, "Device cannot be null");

		ImageSpecInfo image_create_info{};
		image_create_info.size        = m_specInfo.size;
		image_create_info.usage       = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
		image_create_info.mipCount    = m_mipLevels;
		image_create_info.sampleCount = m_specInfo.sampleCount;
		image_create_info.format      = m_specInfo.format;
		m_image                       = make_reference<VKRawImage>(m_device, image_create_info);

		setData(p_data, p_size);
		util::transferDstToShaderRead(m_image.get());
		createSampler();
	}

	VKTexture2D::VKTexture2D(VKLogicalDevice *p_device, const TextureSpecInfo &p_spec_info, const Buffer &p_data) : m_device(p_device), m_specInfo(p_spec_info),
																													m_path(""), m_textureData(p_data)
	{
		TST_ASSERT_MSG(m_device, "Device cannot be null");

		ImageSpecInfo image_create_info{};
		image_create_info.size        = m_specInfo.size;
		image_create_info.usage       = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
		image_create_info.mipCount    = m_mipLevels;
		image_create_info.sampleCount = m_specInfo.sampleCount;
		image_create_info.format      = m_specInfo.format;
		m_image                       = make_reference<VKRawImage>(m_device, image_create_info);

		util::toTransferDst(m_image.get());
		m_image->setData(m_textureData);
		util::transferDstToShaderRead(m_image.get());
		createSampler();
	}

	VKTexture2D::~VKTexture2D()
	{
		m_textureData.release();

		m_device->deferDestruction([device = m_device, sampler = m_sampler]() mutable-> void
		{
			device->destroyObject(sampler);
		});
	}

	auto VKTexture2D::populateWriteDescriptor(vk::WriteDescriptorSet &p_write_descriptor, uint32 p_frame_index) -> void
	{
		p_write_descriptor.pImageInfo = &m_descriptorImageInfo;
		if (!p_write_descriptor.pImageInfo->imageView)
			TST_ASSERT_MSG(false, "Oh no");
	}

	auto VKTexture2D::getDescriptorResourceHandle([[maybe_unused]] uint32 p_frame_index) -> void *
	{
		return m_descriptorImageInfo.imageView;
	}

	auto VKTexture2D::resize(tsm::uint2 p_size) -> void
	{
		m_image->resize(p_size);
		createSampler(vk::ImageLayout::eShaderReadOnlyOptimal);
	}

	auto VKTexture2D::setData(void *p_data, uint64 p_size) -> void
	{
		util::toTransferDst(m_image.get());
		m_textureData.release();
		m_textureData = Buffer::copy(p_data, p_size);
		m_image->setData(m_textureData);
	}

	auto VKTexture2D::setData(const Buffer &p_buffer) -> void
	{
		setData(p_buffer.data(), p_buffer.size());
	}

	auto VKTexture2D::saveToFile(const io::filesystem::Path &p_path) -> void
	{
		m_image->saveToFile(p_path);
	}

	auto VKTexture2D::createSampler(vk::ImageLayout p_override_layout) -> void
	{
		if (m_image->getCurrentImageLayout() == vk::ImageLayout::eTransferDstOptimal)
			util::transferDstToShaderRead(m_image.get());

		// Do not destroy an existing sampler: descriptor sets may still reference it. Only create on first use.
		if (!m_sampler)
		{
			const auto physical_device_props = m_device->getPhysicalDevice()->getVulkanPhysicalDevice().getProperties();

			m_samplerCreateInfo                         = vk::SamplerCreateInfo{};
			m_samplerCreateInfo.magFilter               = m_specInfo.samplerFilter;
			m_samplerCreateInfo.minFilter               = m_specInfo.samplerFilter;
			m_samplerCreateInfo.mipmapMode              = vk::SamplerMipmapMode::eLinear;
			m_samplerCreateInfo.addressModeU            = m_specInfo.samplerAddressMode;
			m_samplerCreateInfo.addressModeV            = m_specInfo.samplerAddressMode;
			m_samplerCreateInfo.addressModeW            = m_specInfo.samplerAddressMode;
			m_samplerCreateInfo.mipLodBias              = 0.0f;
			m_samplerCreateInfo.anisotropyEnable        = true;
			m_samplerCreateInfo.maxAnisotropy           = physical_device_props.limits.maxSamplerAnisotropy;
			m_samplerCreateInfo.compareEnable           = false;
			m_samplerCreateInfo.compareOp               = vk::CompareOp::eAlways;
			m_samplerCreateInfo.minLod                  = 0.0f;
			m_samplerCreateInfo.maxLod                  = vk::LodClampNone;
			m_samplerCreateInfo.borderColor             = vk::BorderColor::eFloatOpaqueWhite;
			m_samplerCreateInfo.unnormalizedCoordinates = false;

			m_sampler = (static_cast<vk::Device>(m_device->getVulkanLogicalDevice())).createSampler(m_samplerCreateInfo);
		}

		// Update descriptor info (image view/layout may change on resize)
		m_descriptorImageInfo.imageLayout = (p_override_layout == vk::ImageLayout::eUndefined) ? m_image->getCurrentImageLayout() : p_override_layout;
		m_descriptorImageInfo.imageView   = m_image->getImageView();
		m_descriptorImageInfo.sampler     = m_sampler;
	}

	auto VKTexture2D::getSpecInfo() const -> const TextureSpecInfo &
	{
		return m_specInfo;
	}

	auto VKTexture2D::getPath() const -> const io::filesystem::Path &
	{
		return m_path;
	}

	auto VKTexture2D::getMipLevelCount() const -> uint32
	{
		return m_mipLevels;
	}

	auto VKTexture2D::getImage() -> RefPtr<VKRawImage>
	{
		return m_image;
	}

	auto VKTexture2D::getImage() const -> const RefPtr<VKRawImage> &
	{
		return m_image;
	}

	auto VKTexture2D::getSampler() -> vk::Sampler &
	{
		return m_sampler;
	}

	auto VKTexture2D::getSamplerCreateInfo() const -> const vk::SamplerCreateInfo &
	{
		return m_samplerCreateInfo;
	}

	auto VKTexture2D::getDescriptorInfo() -> vk::DescriptorImageInfo &
	{
		return m_descriptorImageInfo;
	}

	auto VKTexture2D::getDescriptorInfo() const -> const vk::DescriptorImageInfo &
	{
		return m_descriptorImageInfo;
	}

	VKTexture3D::VKTexture3D(VKLogicalDevice *p_device, const TextureSpecInfo &p_spec_info) : m_device(p_device), m_specInfo(p_spec_info)
	{
		TST_ASSERT_MSG(m_device, "Device cannot be null");

		// const uint64 texture_size{m_specInfo.width * m_specInfo.height * util::getBytesPerPixel(m_specInfo.format) * 6u}

		ImageSpecInfo image_spec_info{};
		image_spec_info.format     = m_specInfo.format;
		image_spec_info.size       = m_specInfo.size;
		image_spec_info.mipCount   = 1u;
		image_spec_info.layerCount = 6u;
		image_spec_info.usage      = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled |
									 vk::ImageUsageFlagBits::eStorage;
		m_image = make_reference<VKRawImage>(m_device, image_spec_info);

		createSampler(vk::ImageLayout::eGeneral);
	}

	VKTexture3D::VKTexture3D(VKLogicalDevice *p_device, const TextureSpecInfo &p_spec_info, const io::filesystem::Path &p_path) : m_device(p_device),
																																  m_specInfo(p_spec_info), m_path(p_path)
	{
		TST_ASSERT_MSG(m_device, "Device cannot be null");

		m_textureData = util::loadTextureIntoBuffer(p_path, m_specInfo.format, m_specInfo.size.x, m_specInfo.size.y);
		if (!m_textureData)
		{
			TST_ASSERT(false);
		}

		ImageSpecInfo image_spec_info{};
		image_spec_info.format     = m_specInfo.format;
		image_spec_info.size       = m_specInfo.size;
		image_spec_info.mipCount   = 1u;
		image_spec_info.layerCount = 6u;
		image_spec_info.usage      = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled |
									 vk::ImageUsageFlagBits::eStorage;
		m_image = make_reference<VKRawImage>(m_device, image_spec_info);

		util::toTransferDst(m_image.get());
		m_image->setData(m_textureData);
		util::transferDstToGeneral(m_image.get());
		createSampler();
	}

	VKTexture3D::VKTexture3D(VKLogicalDevice *p_device, const TextureSpecInfo &p_spec_info, const Buffer &p_data) : m_device(p_device), m_specInfo(p_spec_info)
	{
		TST_ASSERT_MSG(m_device, "Device cannot be null");

		m_textureData = Buffer::copy(p_data);

		ImageSpecInfo image_spec_info{};
		image_spec_info.format     = m_specInfo.format;
		image_spec_info.size       = m_specInfo.size;
		image_spec_info.mipCount   = 1u;
		image_spec_info.layerCount = 6u;
		image_spec_info.usage      = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled |
									 vk::ImageUsageFlagBits::eStorage;
		m_image = make_reference<VKRawImage>(m_device, image_spec_info);

		util::toTransferDst(m_image.get());
		m_image->setData(m_textureData);

		util::transferDstToGeneral(m_image.get());
		createSampler(vk::ImageLayout::eGeneral);
	}

	VKTexture3D::~VKTexture3D()
	{
		m_textureData.release();

		m_device->deferDestruction([device = m_device, sampler = m_sampler]() mutable-> void
		{
			device->destroyObject(sampler);
		});
	}

	auto VKTexture3D::populateWriteDescriptor(vk::WriteDescriptorSet &p_write_descriptor, uint32 p_frame_index) -> void
	{
		p_write_descriptor.pImageInfo = &m_descriptorImageInfo;
		if (!p_write_descriptor.pImageInfo->imageView)
			TST_ASSERT_MSG(false, "Oh no");
	}

	auto VKTexture3D::getDescriptorResourceHandle([[maybe_unused]] uint32 p_frame_index) -> void *
	{
		return m_descriptorImageInfo.imageView;
	}

	auto VKTexture3D::resize(tsm::uint2 p_size) -> void
	{
		m_specInfo.size = p_size;
		m_image->resize(p_size);
		createSampler(vk::ImageLayout::eGeneral);
	}

	auto VKTexture3D::setData(void *p_data, uint64 p_size) -> void
	{
		util::toTransferDst(m_image.get());
		m_textureData.release();
		m_textureData = Buffer::copy(p_data, p_size);
		m_image->setData(m_textureData);
		util::toGeneral(m_image.get());
	}

	auto VKTexture3D::setData(const Buffer &p_buffer) -> void
	{
		setData(p_buffer.data(), p_buffer.size());
	}

	auto VKTexture3D::createSampler(vk::ImageLayout p_override_layout) -> void
	{
		if (m_image->getCurrentImageLayout() == vk::ImageLayout::eTransferDstOptimal)
			util::transferDstToShaderRead(m_image.get());

		// Do not destroy an existing sampler: descriptor sets may still reference it. Only create on first use.
		if (!m_sampler)
		{
			m_sampler = m_device->createSampler(m_specInfo.samplerFilter, m_specInfo.samplerAddressMode);
		}

		// Update descriptor info (image view/layout may change on resize)
		m_descriptorImageInfo.imageLayout = (p_override_layout == vk::ImageLayout::eUndefined) ? m_image->getCurrentImageLayout() : p_override_layout;
		m_descriptorImageInfo.imageView   = m_image->getImageView();
		m_descriptorImageInfo.sampler     = m_sampler;
	}

	auto VKTexture3D::getPath() const -> const io::filesystem::Path &
	{
		return m_path;
	}

	auto VKTexture3D::getSpecInfo() const -> const TextureSpecInfo &
	{
		return m_specInfo;
	}

	auto VKTexture3D::getImage() -> RefPtr<VKRawImage>
	{
		return m_image;
	}

	auto VKTexture3D::getSampler() -> vk::Sampler &
	{
		return m_sampler;
	}

	auto VKTexture3D::getDescriptorInfo() -> vk::DescriptorImageInfo &
	{
		return m_descriptorImageInfo;
	}

	auto VKTexture3D::getDescriptorInfo() const -> const vk::DescriptorImageInfo &
	{
		return m_descriptorImageInfo;
	}

	namespace util
	{
		auto loadTextureIntoBuffer(const io::filesystem::Path &p_path, vk::Format &p_out_format, uint32 &p_out_width, uint32 &p_out_height) -> toaster::Buffer
		{
			Buffer image_data{};

			const bool is_srgb = (p_out_format == vk::Format::eR8G8B8Srgb) || (p_out_format == vk::Format::eR8G8B8A8Srgb);
			int32      width{0u};
			int32      height{0u};
			int32      num_channels{0u};

			if (stbi_is_hdr(p_path.string().c_str()))
			{
				const auto data{reinterpret_cast<uint8 *>(stbi_loadf(p_path.string().c_str(), &width, &height, &num_channels, 4))};
				if (!data)
				{
					DEBUG_LOG_ERROR("Failed to load texture: {}", p_path);
					uint32 fallback_data{0xFF00FFFF};
					image_data.allocate(4u);
					image_data.write(&fallback_data, 4u);
					p_out_format = vk::Format::eR8G8B8A8Unorm;
					p_out_width  = 1u;
					p_out_height = 1u;

					return image_data;
				}
				TST_ASSERT_MSG(width != 0 && height != 0, "Bradar, wat is dis?");
				uint64 size{width * height * 4 * sizeof(float32)};
				image_data.allocate(size);
				image_data.write(data, size);

				p_out_format = vk::Format::eR32G32B32A32Sfloat;

				stbi_image_free(data);
			}
			else
			{
				uint8 *data{stbi_load(p_path.string().c_str(), &width, &height, &num_channels, 4)};
				if (!data)
				{
					DEBUG_LOG_ERROR("Failed to load texture: {}", p_path);

					uint32 fallback_data{0xFF00FFFF};
					image_data.allocate(4u);
					image_data.write(&fallback_data, 4u);
					p_out_format = vk::Format::eR8G8B8A8Unorm;
					p_out_width  = 1u;
					p_out_height = 1u;

					return image_data;
				}
				TST_ASSERT_MSG(width != 0 && height != 0, "Bradar, wat is dis?");
				const uint64 size{width * height * sizeof(uint32)};
				image_data.allocate(size);
				image_data.write(data, size);

				p_out_format = is_srgb ? vk::Format::eR8G8B8A8Srgb : vk::Format::eR8G8B8A8Unorm;

				stbi_image_free(data);
			}

			if (!image_data.data())
			{
				LOG_WARN("Failed to load image: {}", p_path);
				return {};
			}

			p_out_width  = width;
			p_out_height = height;
			return image_data;
		}
	}
}
