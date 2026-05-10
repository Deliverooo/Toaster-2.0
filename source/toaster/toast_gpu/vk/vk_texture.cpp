#include "vk_texture.hpp"

#include <stb/stb_image.h>

#include "vk_logical_device.hpp"

namespace toaster::gpu
{
	VKTexture2D::VKTexture2D(VKLogicalDevice *p_device, const TextureSpecInfo &p_spec_info) : m_device(p_device), m_specInfo(p_spec_info)
	{
		vk::ImageUsageFlags usage_flags{vk::ImageUsageFlagBits::eSampled};

		if (m_specInfo.usage == ETextureUsage::eRenderAttachmentSampled)
		{
			// The only reason to create an image without providing it with any data is to use it as an attachment...
			if (!m_device->isDepthFormat(m_specInfo.format))
			{
				usage_flags |= vk::ImageUsageFlagBits::eColorAttachment;
				if (m_specInfo.sampleCount != vk::SampleCountFlagBits::e1)
					usage_flags |= vk::ImageUsageFlagBits::eTransientAttachment;

				ImageSpecInfo image_create_info{};
				image_create_info.width       = m_specInfo.width;
				image_create_info.height      = m_specInfo.height;
				image_create_info.usage       = usage_flags;
				image_create_info.mipCount    = m_mipLevels;
				image_create_info.sampleCount = m_specInfo.sampleCount;
				image_create_info.format      = m_specInfo.format;
				m_image                       = m_device->alloc<VKRawImage>(image_create_info);
			}
			else
			{
				usage_flags |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
				if (m_specInfo.sampleCount != vk::SampleCountFlagBits::e1)
					usage_flags |= vk::ImageUsageFlagBits::eTransientAttachment;

				ImageSpecInfo image_create_info{};
				image_create_info.width       = m_specInfo.width;
				image_create_info.height      = m_specInfo.height;
				image_create_info.usage       = usage_flags;
				image_create_info.mipCount    = m_mipLevels;
				image_create_info.sampleCount = m_specInfo.sampleCount;
				image_create_info.format      = m_specInfo.format;
				m_image                       = m_device->alloc<VKRawImage>(image_create_info);
			}
		}
		else if (m_specInfo.usage == ETextureUsage::eShaderSampled)
		{
			usage_flags |= vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst;
			ImageSpecInfo image_create_info{};
			image_create_info.width       = m_specInfo.width;
			image_create_info.height      = m_specInfo.height;
			image_create_info.usage       = usage_flags;
			image_create_info.mipCount    = m_mipLevels;
			image_create_info.sampleCount = m_specInfo.sampleCount;
			image_create_info.format      = m_specInfo.format;
			m_image                       = m_device->alloc<VKRawImage>(image_create_info);
		}

		m_sampler = m_device->createSampler();

		m_descriptorImageInfo             = vk::DescriptorImageInfo{};
		m_descriptorImageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		m_descriptorImageInfo.imageView   = m_image->getImageView();
		m_descriptorImageInfo.sampler     = m_sampler;
	}

	VKTexture2D::VKTexture2D(VKLogicalDevice *p_device, const TextureSpecInfo &p_spec_info, const io::filesystem::Path &p_path) : m_device(p_device),
																																  m_specInfo(p_spec_info), m_path(p_path)
	{
		TST_ASSERT_MSG(p_device, "Context cannot be null");

		// Ts is somewhat necessary
		// stbi_set_flip_vertically_on_load(true);

		// Possibly in the future I might look into dynamic colour channels, so the images don't need to be in RGBA
		int32  width{0};
		int32  height{0};
		int32  num_channels{0};
		uint8 *pixels = stbi_load(p_path.string().c_str(), &width, &height, &num_channels, 4);

		bool loaded{true};
		if (!pixels)
		{
			loaded = false;
			LOG_ERROR("failed to load texture image: {}", p_path.string());

			width  = 1;
			height = 1;
			uint32 fallback_data{0xFFFF00FF};
			m_specInfo.format = vk::Format::eR8G8B8A8Unorm;
			pixels            = reinterpret_cast<uint8 *>(&fallback_data);
		}

		vk::DeviceSize image_size = width * height * 4;
		m_specInfo.width          = static_cast<uint32>(width);
		m_specInfo.height         = static_cast<uint32>(height);

		if (m_specInfo.generateMips)
			m_mipLevels = 1u + static_cast<uint32>(std::floor(std::log2(std::max(m_specInfo.width, m_specInfo.height))));
		else
			m_mipLevels = 1;

		ImageSpecInfo image_create_info{};
		image_create_info.width       = m_specInfo.width;
		image_create_info.height      = m_specInfo.height;
		image_create_info.usage       = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
		image_create_info.mipCount    = m_mipLevels;
		image_create_info.sampleCount = m_specInfo.sampleCount;
		image_create_info.format      = m_specInfo.format;
		m_image                       = m_device->alloc<VKRawImage>(image_create_info);

		util::toTransferDst(m_image.get());
		setData(pixels, image_size);
		if (loaded)
			stbi_image_free(pixels);

		m_device->generateMipmaps(m_image->getImage(), {m_specInfo.width, m_specInfo.height, 1u}, m_mipLevels);
		m_image->setCurrentImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal); // Generate mips leaves the image in the eShaderReadOnlyOptimal layout

		createSampler();
	}

	VKTexture2D::VKTexture2D(VKLogicalDevice *p_device, const TextureSpecInfo &p_spec_info, void *p_data, uint64 p_size) : m_device(p_device), m_specInfo(p_spec_info),
																														   m_path("")
	{
		ImageSpecInfo image_create_info{};
		image_create_info.width       = m_specInfo.width;
		image_create_info.height      = m_specInfo.height;
		image_create_info.usage       = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
		image_create_info.mipCount    = m_mipLevels;
		image_create_info.sampleCount = m_specInfo.sampleCount;
		image_create_info.format      = vk::Format::eR8G8B8A8Unorm;
		m_image                       = m_device->alloc<VKRawImage>(image_create_info);

		setData(p_data, p_size);
		util::transferDstToShaderRead(m_image.get());
		createSampler();
	}

	auto VKTexture2D::resize(uint32 p_width, uint32 p_height) -> void
	{
		m_image->resize(p_width, p_height);
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

	auto VKTexture2D::createSampler(vk::ImageLayout p_override_layout) -> void
	{
		if (m_image->getCurrentImageLayout() == vk::ImageLayout::eTransferDstOptimal)
			util::transferDstToShaderRead(m_image.get());

		m_sampler             = nullptr;
		m_descriptorImageInfo = vk::DescriptorImageInfo{};

		m_sampler = m_device->createSampler();

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

	auto VKTexture2D::getSampler() -> vk::raii::Sampler &
	{
		return m_sampler;
	}

	auto VKTexture2D::getDescriptorInfo() -> vk::DescriptorImageInfo &
	{
		return m_descriptorImageInfo;
	}

	VKTexture3D::VKTexture3D(VKLogicalDevice *p_device, const TextureSpecInfo &p_spec_info, Buffer p_data) : m_device(p_device), m_specInfo(p_spec_info)
	{
		constexpr uint32 colour_channels{4u};
		uint32           size{m_specInfo.width * m_specInfo.height * colour_channels * 6};

		m_textureData = Buffer::copy(p_data);

		ImageSpecInfo image_spec_info{};
		image_spec_info.format     = m_specInfo.format;
		image_spec_info.width      = m_specInfo.width;
		image_spec_info.height     = m_specInfo.height;
		image_spec_info.mipCount   = 1u;
		image_spec_info.layerCount = 6u;
		image_spec_info.usage      = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled |
									 vk::ImageUsageFlagBits::eStorage;

		m_image = m_device->alloc<VKRawImage>(image_spec_info);

		util::toTransferDst(m_image.get());
		m_image->setData(m_textureData);

		util::transferDstToGeneral(m_image.get());
		createSampler(vk::ImageLayout::eGeneral);

		if (m_image->getCurrentImageLayout() == vk::ImageLayout::eUndefined)
		{
			TST_ASSERT(false);
		}
	}

	auto VKTexture3D::resize(uint32 p_width, uint32 p_height) -> void
	{
		m_image->resize(p_width, p_height);
		createSampler(vk::ImageLayout::eGeneral);
	}

	auto VKTexture3D::setData(void *p_data, uint64 p_size) -> void
	{
		util::toTransferDst(m_image.get());
		m_textureData.release();
		m_textureData = Buffer::copy(p_data, p_size);
		m_image->setData(m_textureData);
	}

	auto VKTexture3D::setData(const Buffer &p_buffer) -> void
	{
		setData(p_buffer.data(), p_buffer.size());
	}

	auto VKTexture3D::createSampler(vk::ImageLayout p_override_layout) -> void
	{
		if (m_image->getCurrentImageLayout() == vk::ImageLayout::eTransferDstOptimal)
			util::transferDstToShaderRead(m_image.get());

		m_sampler             = nullptr;
		m_descriptorImageInfo = vk::DescriptorImageInfo{};

		m_sampler = m_device->createSampler();

		m_descriptorImageInfo.imageLayout = (p_override_layout == vk::ImageLayout::eUndefined) ? m_image->getCurrentImageLayout() : p_override_layout;
		m_descriptorImageInfo.imageView   = m_image->getImageView();
		m_descriptorImageInfo.sampler     = m_sampler;
	}

	auto VKTexture3D::getSpecInfo() const -> const TextureSpecInfo &
	{
		return m_specInfo;
	}

	auto VKTexture3D::getImage() -> RefPtr<VKRawImage>
	{
		return m_image;
	}

	auto VKTexture3D::getSampler() -> vk::raii::Sampler &
	{
		return m_sampler;
	}

	auto VKTexture3D::getDescriptorInfo() -> vk::DescriptorImageInfo &
	{
		return m_descriptorImageInfo;
	}
}
