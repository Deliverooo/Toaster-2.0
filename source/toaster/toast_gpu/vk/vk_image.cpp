#include "vk_image.hpp"

#include "vk_logical_device.hpp"

namespace toaster::gpu
{
	VKImage2D::VKImage2D(VKLogicalDevice *p_dev, const ImageCreateInfo &p_create_info) : m_device(p_dev), m_createInfo(p_create_info)
	{
		TST_ASSERT_MSG(p_dev, "Device cannot be null");

		recreate();
	}

	auto VKImage2D::getDevice() const -> VKLogicalDevice *
	{
		return m_device;
	}

	auto VKImage2D::getImage() -> vk::raii::Image &
	{
		return m_image;
	}

	auto VKImage2D::getImageMemory() -> vk::raii::DeviceMemory &
	{
		return m_imageMemory;
	}

	auto VKImage2D::getImageView() -> vk::raii::ImageView &
	{
		return m_imageView;
	}

	auto VKImage2D::getCreateInfo() const -> const ImageCreateInfo &
	{
		return m_createInfo;
	}

	auto VKImage2D::setCurrentImageLayout(vk::ImageLayout p_layout) -> void
	{
		m_currentImageLayout = p_layout;
	}

	auto VKImage2D::getCurrentImageLayout() const -> vk::ImageLayout
	{
		return m_currentImageLayout;
	}

	auto VKImage2D::resize(uint32 p_width, uint32 p_height) -> void
	{
		m_createInfo.width  = p_width;
		m_createInfo.height = p_height;

		recreate();
	}

	auto VKImage2D::recreate() -> void
	{
		m_image       = nullptr;
		m_imageMemory = nullptr;
		m_imageView   = nullptr;

		m_currentImageLayout = vk::ImageLayout::eUndefined;

		m_device->createImage(m_createInfo.width, m_createInfo.height, m_createInfo.mipCount, m_createInfo.sampleCount, m_createInfo.format, vk::ImageTiling::eOptimal,
							  m_createInfo.usage, vk::MemoryPropertyFlagBits::eDeviceLocal, m_image, m_imageMemory);

		vk::ImageAspectFlags aspect_flags{m_device->isDepthFormat(m_createInfo.format) ? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor};
		if (m_device->hasStencilComponent(m_createInfo.format))
			aspect_flags |= vk::ImageAspectFlagBits::eStencil;

		if (m_createInfo.usage & vk::ImageUsageFlagBits::eColorAttachment)
		{
			m_device->transitionImageLayout(m_image, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, vk::AccessFlagBits2::eNone,
											vk::AccessFlagBits2::eColorAttachmentWrite, vk::PipelineStageFlagBits2::eNone,
											vk::PipelineStageFlagBits2::eColorAttachmentOutput, m_createInfo.mipCount, aspect_flags);
			m_currentImageLayout = vk::ImageLayout::eColorAttachmentOptimal;
		}
		else if (m_createInfo.usage & vk::ImageUsageFlagBits::eDepthStencilAttachment)
		{
			m_device->transitionImageLayout(m_image, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthAttachmentOptimal, vk::AccessFlagBits2::eNone,
											vk::AccessFlagBits2::eDepthStencilAttachmentWrite, vk::PipelineStageFlagBits2::eNone,
											vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests, m_createInfo.mipCount,
											aspect_flags);
			m_currentImageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
		}

		m_imageView = m_device->createImageView(m_image, m_createInfo.format, aspect_flags, m_createInfo.mipCount);
	}
}
