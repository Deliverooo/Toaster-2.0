#include "vk_storage_image.hpp"

#include "vk_logical_device.hpp"

namespace toaster::gpu
{
	VKStorageImage::VKStorageImage(VKLogicalDevice *p_device, const ImageSpecInfo &p_spec_info) : m_device(p_device)
	{
		m_image = make_reference<VKRawImage>(m_device, p_spec_info);
		util::undefinedToGeneral(m_image.get());
		createSampler(vk::ImageLayout::eGeneral);
	}

	VKStorageImage::~VKStorageImage()
	{
		m_device->deferDestruction([device = m_device, sampler = m_sampler]() mutable-> void
		{
			device->destroyObject(sampler);
		});
	}

	auto VKStorageImage::populateWriteDescriptor(vk::WriteDescriptorSet &p_write_descriptor) -> void
	{
		p_write_descriptor.pImageInfo = &m_descriptorImageInfo;
		if (!p_write_descriptor.pImageInfo->imageView)
			TST_ASSERT_MSG(false, "Oh no");
	}

	auto VKStorageImage::getDescriptorResourceHandle([[maybe_unused]] uint32 p_frame_index) -> void *
	{
		return m_descriptorImageInfo.imageView;
	}

	auto VKStorageImage::resize(uint32 p_width, uint32 p_height) -> void
	{
		m_image->resize(p_width, p_height);
		util::undefinedToGeneral(m_image.get());
		createSampler(vk::ImageLayout::eGeneral);
	}

	auto VKStorageImage::setData(void *p_data, uint64 p_size) -> void
	{
		m_imageData.release();
		m_imageData.allocate(p_size);
		m_imageData = Buffer::copy(p_data, p_size);
		m_image->setData(p_data, p_size);
	}

	auto VKStorageImage::setData(const Buffer &p_buffer) -> void
	{
		setData(p_buffer.data(), p_buffer.size());
	}

	auto VKStorageImage::createSampler(vk::ImageLayout p_override_layout) -> void
	{
		if (m_image->getCurrentImageLayout() == vk::ImageLayout::eTransferDstOptimal)
			util::transferDstToShaderRead(m_image.get());

		m_device->destroyObject<vk::Sampler>(m_sampler);
		m_sampler             = nullptr;
		m_descriptorImageInfo = vk::DescriptorImageInfo{};

		m_sampler = m_device->createSampler();

		m_descriptorImageInfo.imageLayout = (p_override_layout == vk::ImageLayout::eUndefined) ? m_image->getCurrentImageLayout() : p_override_layout;
		m_descriptorImageInfo.imageView   = m_image->getImageView();
		m_descriptorImageInfo.sampler     = m_sampler;
	}

	auto VKStorageImage::getImage() const -> const RawImageHandle &
	{
		return m_image;
	}

	auto VKStorageImage::getDescriptorInfo() const -> const vk::DescriptorImageInfo &
	{
		return m_descriptorImageInfo;
	}
}
