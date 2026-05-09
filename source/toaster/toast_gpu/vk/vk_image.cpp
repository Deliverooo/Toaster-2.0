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

		vk::ImageTiling image_tiling{(m_specInfo.usage & vk::ImageUsageFlagBits::eStorage) ? vk::ImageTiling::eLinear : vk::ImageTiling::eOptimal};

		m_device->createImage(m_specInfo.width, m_specInfo.height, m_specInfo.mipCount, m_specInfo.sampleCount, m_specInfo.format, image_tiling, m_specInfo.usage,
							  vk::MemoryPropertyFlagBits::eDeviceLocal, m_image, m_imageMemory);

		vk::ImageAspectFlags aspect_flags{m_device->isDepthFormat(m_specInfo.format) ? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor};
		if (m_device->hasStencilComponent(m_specInfo.format))
			aspect_flags |= vk::ImageAspectFlagBits::eStencil;

		if (m_specInfo.usage & vk::ImageUsageFlagBits::eColorAttachment)
		{
			util::undefinedToColourAttachment(this);
		}
		else if (m_specInfo.usage & vk::ImageUsageFlagBits::eDepthStencilAttachment)
		{
			util::undefinedToDepthAttachment(this, false);
		}
		else if (m_specInfo.usage & vk::ImageUsageFlagBits::eStorage)
		{
			m_currentImageLayout = vk::ImageLayout::eGeneral;
		}

		m_imageView = m_device->createImageView(m_image, m_specInfo.format, aspect_flags, m_specInfo.mipCount);
	}

	VKImage2D::VKImage2D(VKLogicalDevice *p_device, const ImageSpecInfo &p_spec_info) : m_device(p_device)
	{
		m_image = m_device->alloc<VKRawImage>(p_spec_info);
		util::undefinedToGeneral(m_image.get());
		createSampler(vk::ImageLayout::eGeneral);
	}

	auto VKImage2D::resize(uint32 p_width, uint32 p_height) -> void
	{
		m_image->resize(p_width, p_height);
		util::undefinedToGeneral(m_image.get());
		createSampler(vk::ImageLayout::eGeneral);
	}

	auto VKImage2D::setData(void *p_data, uint64 p_size) -> void
	{
		m_imageData.release();
		m_imageData.allocate(p_size);
		m_imageData = Buffer::copy(p_data, p_size);

		vk::raii::Buffer       staging_buffer{nullptr};
		vk::raii::DeviceMemory staging_buffer_memory{nullptr};

		m_device->createBuffer(m_imageData.size(), vk::BufferUsageFlagBits::eTransferSrc,
							   vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, staging_buffer, staging_buffer_memory);

		void *mapped = staging_buffer_memory.mapMemory(0, m_imageData.size(), {});
		std::memcpy(mapped, m_imageData.data(), m_imageData.size());
		staging_buffer_memory.unmapMemory();

		util::undefinedToTransferDst(m_image.get());
		m_device->copyBufferToImage(staging_buffer, m_image->getImage(), m_image->getSpecInfo().width, m_image->getSpecInfo().height);
	}

	auto VKImage2D::createSampler(vk::ImageLayout p_override_layout) -> void
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

	auto VKImage2D::getImage() const -> const RefPtr<VKRawImage> &
	{
		return m_image;
	}

	auto VKImage2D::getDescriptorInfo() const -> const vk::DescriptorImageInfo &
	{
		return m_descriptorImageInfo;
	}

	namespace util
	{
		auto shaderReadToColourAttachment(AttachmentImage *p_image) -> void
		{
			p_image->getDevice()->transitionImageLayout(p_image->getImage(), vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eColorAttachmentOptimal,
														vk::AccessFlagBits2::eShaderRead, vk::AccessFlagBits2::eColorAttachmentWrite,
														vk::PipelineStageFlagBits2::eFragmentShader, vk::PipelineStageFlagBits2::eColorAttachmentOutput,
														p_image->getSpecInfo().mipCount, vk::ImageAspectFlagBits::eColor);
			p_image->setCurrentImageLayout(vk::ImageLayout::eColorAttachmentOptimal);
		}

		auto shaderReadToDepthAttachment(AttachmentImage *p_image, bool p_read_only) -> void
		{
			const vk::ImageLayout  new_layout{p_read_only ? vk::ImageLayout::eDepthReadOnlyOptimal : vk::ImageLayout::eDepthAttachmentOptimal};
			const vk::AccessFlags2 dst_access_flags{p_read_only ? vk::AccessFlagBits2::eDepthStencilAttachmentRead : vk::AccessFlagBits2::eDepthStencilAttachmentWrite};
			p_image->getDevice()->transitionImageLayout(p_image->getImage(), vk::ImageLayout::eShaderReadOnlyOptimal, new_layout, vk::AccessFlagBits2::eShaderRead,
														dst_access_flags, vk::PipelineStageFlagBits2::eFragmentShader,
														vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
														p_image->getSpecInfo().mipCount, vk::ImageAspectFlagBits::eDepth);

			p_image->setCurrentImageLayout(new_layout);
		}

		auto shaderReadToTransferSrc(AttachmentImage *p_image) -> void
		{
			p_image->getDevice()->transitionImageLayout(p_image->getImage(), vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eTransferSrcOptimal,
														vk::AccessFlagBits2::eShaderRead, vk::AccessFlagBits2::eTransferRead, vk::PipelineStageFlagBits2::eFragmentShader,
														vk::PipelineStageFlagBits2::eTransfer, p_image->getSpecInfo().mipCount, vk::ImageAspectFlagBits::eColor);
			p_image->setCurrentImageLayout(vk::ImageLayout::eTransferSrcOptimal);
		}

		auto shaderReadToTransferDst(AttachmentImage *p_image) -> void
		{
			p_image->getDevice()->transitionImageLayout(p_image->getImage(), vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eTransferDstOptimal,
														vk::AccessFlagBits2::eShaderRead, vk::AccessFlagBits2::eTransferWrite,
														vk::PipelineStageFlagBits2::eFragmentShader, vk::PipelineStageFlagBits2::eTransfer,
														p_image->getSpecInfo().mipCount, vk::ImageAspectFlagBits::eColor);
			p_image->setCurrentImageLayout(vk::ImageLayout::eTransferDstOptimal);
		}

		auto colourAttachmentToShaderRead(AttachmentImage *p_image) -> void
		{
			p_image->getDevice()->transitionImageLayout(p_image->getImage(), vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
														vk::AccessFlagBits2::eColorAttachmentWrite, vk::AccessFlagBits2::eShaderRead,
														vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eFragmentShader,
														p_image->getSpecInfo().mipCount, vk::ImageAspectFlagBits::eColor);
			p_image->setCurrentImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
		}

		auto depthAttachmentToShaderRead(AttachmentImage *p_image, bool p_read_only) -> void
		{
			const vk::ImageLayout  old_layout{p_read_only ? vk::ImageLayout::eDepthReadOnlyOptimal : vk::ImageLayout::eDepthAttachmentOptimal};
			const vk::AccessFlags2 src_access_flags{p_read_only ? vk::AccessFlagBits2::eDepthStencilAttachmentRead : vk::AccessFlagBits2::eDepthStencilAttachmentWrite};

			p_image->getDevice()->transitionImageLayout(p_image->getImage(), old_layout, vk::ImageLayout::eShaderReadOnlyOptimal, src_access_flags,
														vk::AccessFlagBits2::eShaderRead,
														vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
														vk::PipelineStageFlagBits2::eFragmentShader, p_image->getSpecInfo().mipCount, vk::ImageAspectFlagBits::eDepth);
			p_image->setCurrentImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
		}

		auto transferSrcToShaderRead(AttachmentImage *p_image) -> void
		{
			p_image->getDevice()->transitionImageLayout(p_image->getImage(), vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
														vk::AccessFlagBits2::eTransferRead, vk::AccessFlagBits2::eShaderRead, vk::PipelineStageFlagBits2::eTransfer,
														vk::PipelineStageFlagBits2::eFragmentShader, p_image->getSpecInfo().mipCount, vk::ImageAspectFlagBits::eColor);
			p_image->setCurrentImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
		}

		auto transferDstToShaderRead(AttachmentImage *p_image) -> void
		{
			p_image->getDevice()->transitionImageLayout(p_image->getImage(), vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
														vk::AccessFlagBits2::eTransferWrite, vk::AccessFlagBits2::eShaderRead, vk::PipelineStageFlagBits2::eTransfer,
														vk::PipelineStageFlagBits2::eFragmentShader, p_image->getSpecInfo().mipCount, vk::ImageAspectFlagBits::eColor);
			p_image->setCurrentImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
		}

		auto undefinedToColourAttachment(AttachmentImage *p_image) -> void
		{
			p_image->getDevice()->transitionImageLayout(p_image->getImage(), vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal,
														vk::AccessFlagBits2::eNone, vk::AccessFlagBits2::eColorAttachmentWrite, vk::PipelineStageFlagBits2::eTopOfPipe,
														vk::PipelineStageFlagBits2::eColorAttachmentOutput, p_image->getSpecInfo().mipCount,
														vk::ImageAspectFlagBits::eColor);
			p_image->setCurrentImageLayout(vk::ImageLayout::eColorAttachmentOptimal);
		}

		auto undefinedToDepthAttachment(AttachmentImage *p_image, bool p_read_only) -> void
		{
			const vk::ImageLayout  new_layout{p_read_only ? vk::ImageLayout::eDepthReadOnlyOptimal : vk::ImageLayout::eDepthAttachmentOptimal};
			const vk::AccessFlags2 dst_access_flags{p_read_only ? vk::AccessFlagBits2::eDepthStencilAttachmentRead : vk::AccessFlagBits2::eDepthStencilAttachmentWrite};

			p_image->getDevice()->transitionImageLayout(p_image->getImage(), vk::ImageLayout::eUndefined, new_layout, vk::AccessFlagBits2::eNone, dst_access_flags,
														vk::PipelineStageFlagBits2::eTopOfPipe,
														vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
														p_image->getSpecInfo().mipCount, vk::ImageAspectFlagBits::eDepth);
			p_image->setCurrentImageLayout(new_layout);
		}

		auto undefinedToTransferSrc(AttachmentImage *p_image) -> void
		{
			p_image->getDevice()->transitionImageLayout(p_image->getImage(), vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferSrcOptimal,
														vk::AccessFlagBits2::eNone, vk::AccessFlagBits2::eTransferRead, vk::PipelineStageFlagBits2::eTopOfPipe,
														vk::PipelineStageFlagBits2::eTransfer, p_image->getSpecInfo().mipCount, vk::ImageAspectFlagBits::eColor);
			p_image->setCurrentImageLayout(vk::ImageLayout::eTransferSrcOptimal);
		}

		auto undefinedToTransferDst(AttachmentImage *p_image) -> void
		{
			p_image->getDevice()->transitionImageLayout(p_image->getImage(), vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
														vk::AccessFlagBits2::eNone, vk::AccessFlagBits2::eTransferWrite, vk::PipelineStageFlagBits2::eTopOfPipe,
														vk::PipelineStageFlagBits2::eTransfer, p_image->getSpecInfo().mipCount, vk::ImageAspectFlagBits::eColor);
			p_image->setCurrentImageLayout(vk::ImageLayout::eTransferDstOptimal);
		}

		auto undefinedToGeneral(AttachmentImage *p_image) -> void
		{
			p_image->getDevice()->transitionImageLayout(p_image->getImage(), vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
														vk::AccessFlagBits2::eNone, vk::AccessFlagBits2::eShaderRead | vk::AccessFlagBits2::eShaderWrite, vk::PipelineStageFlagBits2::eTopOfPipe,
														vk::PipelineStageFlagBits2::eComputeShader | vk::PipelineStageFlagBits2::eFragmentShader, p_image->getSpecInfo().mipCount, vk::ImageAspectFlagBits::eColor);
			p_image->setCurrentImageLayout(vk::ImageLayout::eGeneral);
		}
	}
}
