#include "vk_image.hpp"

#include "vk_logical_device.hpp"

namespace toaster::gpu
{
	VKRawImage::VKRawImage(VKLogicalDevice *p_dev, const ImageSpecInfo &p_spec_info) : m_device(p_dev), m_specInfo(p_spec_info)
	{
		TST_ASSERT_MSG(p_dev, "Device cannot be null");

		recreate();
	}

	auto VKRawImage::getImage() -> vk::raii::Image &
	{
		return m_image;
	}

	auto VKRawImage::getImageMemory() -> vk::raii::DeviceMemory &
	{
		return m_imageMemory;
	}

	auto VKRawImage::getImageView() -> vk::raii::ImageView &
	{
		return m_imageView;
	}

	auto VKRawImage::getSpecInfo() const -> const ImageSpecInfo &
	{
		return m_specInfo;
	}

	auto VKRawImage::setCurrentImageLayout(vk::ImageLayout p_layout) -> void
	{
		m_currentImageLayout = p_layout;
	}

	auto VKRawImage::getCurrentImageLayout() const -> vk::ImageLayout
	{
		return m_currentImageLayout;
	}

	auto VKRawImage::resize(uint32 p_width, uint32 p_height) -> void
	{
		m_specInfo.width  = p_width;
		m_specInfo.height = p_height;

		recreate();
	}

	auto VKRawImage::recreate() -> void
	{
		m_image       = nullptr;
		m_imageMemory = nullptr;
		m_imageView   = nullptr;

		m_currentImageLayout = vk::ImageLayout::eUndefined;

		m_device->createImage(m_specInfo.width, m_specInfo.height, m_specInfo.mipCount, m_specInfo.sampleCount, m_specInfo.format, vk::ImageTiling::eOptimal,
							  m_specInfo.usage, vk::MemoryPropertyFlagBits::eDeviceLocal, m_image, m_imageMemory);

		vk::ImageAspectFlags aspect_flags{m_device->isDepthFormat(m_specInfo.format) ? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor};
		if (m_device->hasStencilComponent(m_specInfo.format))
			aspect_flags |= vk::ImageAspectFlagBits::eStencil;

		if (m_specInfo.usage & vk::ImageUsageFlagBits::eColorAttachment)
		{
			m_device->transitionImageLayout(m_image, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, vk::AccessFlagBits2::eNone,
											vk::AccessFlagBits2::eColorAttachmentWrite, vk::PipelineStageFlagBits2::eNone,
											vk::PipelineStageFlagBits2::eColorAttachmentOutput, m_specInfo.mipCount, aspect_flags);
			m_currentImageLayout = vk::ImageLayout::eColorAttachmentOptimal;
		}
		else if (m_specInfo.usage & vk::ImageUsageFlagBits::eDepthStencilAttachment)
		{
			m_device->transitionImageLayout(m_image, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthAttachmentOptimal, vk::AccessFlagBits2::eNone,
											vk::AccessFlagBits2::eDepthStencilAttachmentWrite, vk::PipelineStageFlagBits2::eNone,
											vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests, m_specInfo.mipCount,
											aspect_flags);
			m_currentImageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
		}

		m_imageView = m_device->createImageView(m_image, m_specInfo.format, aspect_flags, m_specInfo.mipCount);
	}

	VKImage2D::VKImage2D(VKLogicalDevice *p_device, const RefPtr<VKRawImage> &p_image) : m_device(p_device), m_image(p_image)
	{
		TST_ASSERT_MSG(m_image, "Image cannot be null");

		m_descriptorImageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		m_descriptorImageInfo.imageView   = m_image->getImageView();
		m_descriptorImageInfo.sampler     = nullptr;
	}

	auto VKImage2D::getImage() const -> const RefPtr<VKRawImage> &
	{
		return m_image;
	}

	auto VKImage2D::getDescriptorInfo() const -> const vk::DescriptorImageInfo &
	{
		return m_descriptorImageInfo;
	}
}
