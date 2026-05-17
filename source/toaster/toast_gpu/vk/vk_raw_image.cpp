#include "vk_raw_image.hpp"

#include "vk_logical_device.hpp"
#include "stb/stb_image_write.h"

namespace toaster::gpu
{
	VKRawImage::VKRawImage(VKLogicalDevice *p_device, const ImageSpecInfo &p_spec_info) : m_device(p_device), m_specInfo(p_spec_info)
	{
		TST_ASSERT_MSG(p_device, "Device cannot be null");

		recreate();
	}

	VKRawImage::~VKRawImage()
	{
		m_device->deferDestruction([device = m_device, image = m_image, image_memory = m_imageMemory, image_view = m_imageView]()mutable -> void
		{
			device->destroyObject(image);
			device->destroyObject(image_memory);
			device->destroyObject(image_view);
		});
	}

	auto VKRawImage::getImage() -> vk::Image &
	{
		return m_image;
	}

	auto VKRawImage::getImageMemory() -> vk::DeviceMemory &
	{
		return m_imageMemory;
	}

	auto VKRawImage::getImageView() -> vk::ImageView &
	{
		return m_imageView;
	}

	auto VKRawImage::getSpecInfo() const -> const ImageSpecInfo &
	{
		return m_specInfo;
	}

	auto VKRawImage::isMultisample() const -> bool
	{
		return m_specInfo.sampleCount != vk::SampleCountFlagBits::e1;
	}

	auto VKRawImage::setCurrentImageLayout(vk::ImageLayout p_layout) -> void
	{
		m_currentImageLayout = p_layout;
	}

	auto VKRawImage::getCurrentImageLayout() const -> vk::ImageLayout
	{
		return m_currentImageLayout;
	}

	auto VKRawImage::saveToFile(const io::filesystem::Path &p_path) -> void
	{
		vk::raii::Buffer       staging_buffer{nullptr};
		vk::raii::DeviceMemory staging_buffer_memory{nullptr};

		const uint64 buffer_size{util::getBytesPerPixel(m_specInfo.format) * m_specInfo.width * m_specInfo.height};
		m_device->createBuffer(buffer_size, vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
							   staging_buffer, staging_buffer_memory);

		vk::ImageLayout previous_layout{m_currentImageLayout};
		util::transitionImageLayout(this, m_currentImageLayout, vk::ImageLayout::eTransferSrcOptimal);
		m_device->copyImageToBuffer(m_image, staging_buffer, {m_specInfo.width, m_specInfo.height, 1u}, m_specInfo.layerCount);
		util::transitionImageLayout(this, vk::ImageLayout::eTransferSrcOptimal, previous_layout);

		void *image_data{staging_buffer_memory.mapMemory(0u, buffer_size)};
		int32 success{stbi_write_bmp(p_path.string().c_str(), m_specInfo.width, m_specInfo.height, 4, image_data)};
		staging_buffer_memory.unmapMemory();

		TST_PERMA_ASSERT_MSG(success, "No");
	}

	auto VKRawImage::setData(void *p_data, uint64 p_size) -> void
	{
		vk::raii::Buffer       staging_buffer{nullptr};
		vk::raii::DeviceMemory staging_buffer_memory{nullptr};

		m_device->createBuffer(p_size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
							   staging_buffer, staging_buffer_memory);

		void *mapped{staging_buffer_memory.mapMemory(0, p_size, {})};
		std::memcpy(mapped, p_data, p_size);
		staging_buffer_memory.unmapMemory();

		util::transitionImageLayout(this, m_currentImageLayout, vk::ImageLayout::eTransferDstOptimal);
		m_device->copyBufferToImage(staging_buffer, m_image, {m_specInfo.width, m_specInfo.height, 1u}, m_specInfo.layerCount);
	}

	auto VKRawImage::setData(const Buffer &p_buffer) -> void
	{
		setData(p_buffer.data(), p_buffer.size());
	}

	auto VKRawImage::resize(uint32 p_width, uint32 p_height) -> void
	{
		m_specInfo.width  = p_width;
		m_specInfo.height = p_height;

		recreate();
	}

	auto VKRawImage::recreate() -> void
	{
		m_device->deferDestruction([device = m_device, image = m_image, image_memory = m_imageMemory, image_view = m_imageView]() mutable-> void
		{
			device->destroyObject<vk::Image>(image);
			device->destroyObject<vk::DeviceMemory>(image_memory);
			device->destroyObject<vk::ImageView>(image_view);
		});
		m_image       = nullptr;
		m_imageMemory = nullptr;
		m_imageView   = nullptr;

		m_currentImageLayout = vk::ImageLayout::eUndefined;

		auto image_tiling{vk::ImageTiling::eOptimal};
		if (m_specInfo.usage & vk::ImageUsageFlagBits::eStorage)
			image_tiling = vk::ImageTiling::eLinear;
		if (m_specInfo.layerCount > 1)
			image_tiling = vk::ImageTiling::eOptimal;

		m_device->createImage({m_specInfo.width, m_specInfo.height, 1u}, m_specInfo.layerCount, m_specInfo.mipCount, m_specInfo.sampleCount, m_specInfo.format,
							  image_tiling, m_specInfo.usage, vk::MemoryPropertyFlagBits::eDeviceLocal, m_image, m_imageMemory);

		const vk::ImageAspectFlags aspect_flags{util::getImageAspectMask(m_specInfo.format)};
		if (m_specInfo.usage & vk::ImageUsageFlagBits::eColorAttachment)
			util::transitionImageLayout(this, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);
		else if (m_specInfo.usage & vk::ImageUsageFlagBits::eDepthStencilAttachment)
			util::transitionImageLayout(this, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthAttachmentOptimal);
		else if (m_specInfo.usage & vk::ImageUsageFlagBits::eStorage)
			util::transitionImageLayout(this, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral);

		m_imageView = m_device->createImageView(m_image, m_specInfo.format, aspect_flags, m_specInfo.layerCount, m_specInfo.mipCount);
	}

	namespace util
	{
		auto transitionImageLayout(VKRawImage *p_image, vk::ImageLayout p_src_layout, vk::ImageLayout p_dst_layout, vk::CommandBuffer p_override_command_buffer) -> void
		{
			TST_PERMA_ASSERT_MSG(p_image->getCurrentImageLayout() == p_src_layout, "Image is not in specified source layout!");
			p_image->getDevice()->transitionImageLayout(p_image->getImage(), getImageLayoutInfo(p_src_layout), getImageLayoutInfo(p_dst_layout),
														p_image->getSpecInfo().layerCount, p_image->getSpecInfo().mipCount,
														getImageAspectMask(p_image->getSpecInfo().format), p_override_command_buffer);
			p_image->setCurrentImageLayout(p_dst_layout);
		}

		auto colourAttachmentToShaderRead(VKRawImage *p_image) -> void
		{
			p_image->getDevice()->transitionImageLayout(p_image->getImage(), getImageLayoutInfo(vk::ImageLayout::eColorAttachmentOptimal),
														getImageLayoutInfo(vk::ImageLayout::eShaderReadOnlyOptimal), p_image->getSpecInfo().layerCount,
														p_image->getSpecInfo().mipCount, vk::ImageAspectFlagBits::eColor);
			p_image->setCurrentImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
		}

		auto colourAttachmentToTransferSrc(VKRawImage *p_image) -> void
		{
			p_image->getDevice()->transitionImageLayout(p_image->getImage(), getImageLayoutInfo(vk::ImageLayout::eColorAttachmentOptimal),
														getImageLayoutInfo(vk::ImageLayout::eTransferSrcOptimal), p_image->getSpecInfo().layerCount,
														p_image->getSpecInfo().mipCount, vk::ImageAspectFlagBits::eColor);
			p_image->setCurrentImageLayout(vk::ImageLayout::eTransferSrcOptimal);
		}

		auto colourAttachmentToTransferDst(VKRawImage *p_image) -> void
		{
			p_image->getDevice()->transitionImageLayout(p_image->getImage(), getImageLayoutInfo(vk::ImageLayout::eColorAttachmentOptimal),
														getImageLayoutInfo(vk::ImageLayout::eTransferDstOptimal), p_image->getSpecInfo().layerCount,
														p_image->getSpecInfo().mipCount, vk::ImageAspectFlagBits::eColor);
			p_image->setCurrentImageLayout(vk::ImageLayout::eTransferDstOptimal);
		}

		auto colourAttachmentToGeneral(VKRawImage *p_image) -> void
		{
			p_image->getDevice()->transitionImageLayout(p_image->getImage(), getImageLayoutInfo(vk::ImageLayout::eColorAttachmentOptimal),
														getImageLayoutInfo(vk::ImageLayout::eGeneral), p_image->getSpecInfo().layerCount, p_image->getSpecInfo().mipCount,
														vk::ImageAspectFlagBits::eColor);
			p_image->setCurrentImageLayout(vk::ImageLayout::eGeneral);
		}

		auto depthAttachmentToShaderRead(VKRawImage *p_image, bool p_read_only) -> void
		{
			const vk::ImageLayout  old_layout{p_read_only ? vk::ImageLayout::eDepthReadOnlyOptimal : vk::ImageLayout::eDepthAttachmentOptimal};
			const vk::AccessFlags2 src_access_flags{p_read_only ? vk::AccessFlagBits2::eDepthStencilAttachmentRead : vk::AccessFlagBits2::eDepthStencilAttachmentWrite};

			p_image->getDevice()->transitionImageLayout(p_image->getImage(), old_layout, vk::ImageLayout::eShaderReadOnlyOptimal, src_access_flags,
														vk::AccessFlagBits2::eShaderRead,
														vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
														vk::PipelineStageFlagBits2::eFragmentShader, p_image->getSpecInfo().layerCount, p_image->getSpecInfo().mipCount,
														vk::ImageAspectFlagBits::eDepth);
			p_image->setCurrentImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
		}

		auto depthAttachmentToTransferSrc(VKRawImage *p_image, bool p_read_only) -> void
		{
		}

		auto depthAttachmentToTransferDst(VKRawImage *p_image, bool p_read_only) -> void
		{
		}

		auto depthAttachmentToGeneral(VKRawImage *p_image, bool p_read_only) -> void
		{
		}

		auto shaderReadToColourAttachment(VKRawImage *p_image) -> void
		{
			p_image->getDevice()->transitionImageLayout(p_image->getImage(), vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eColorAttachmentOptimal,
														vk::AccessFlagBits2::eShaderRead, vk::AccessFlagBits2::eColorAttachmentWrite,
														vk::PipelineStageFlagBits2::eFragmentShader, vk::PipelineStageFlagBits2::eColorAttachmentOutput,
														p_image->getSpecInfo().layerCount, p_image->getSpecInfo().mipCount, vk::ImageAspectFlagBits::eColor);
			p_image->setCurrentImageLayout(vk::ImageLayout::eColorAttachmentOptimal);
		}

		auto shaderReadToDepthAttachment(VKRawImage *p_image, bool p_read_only) -> void
		{
			const vk::ImageLayout  new_layout{p_read_only ? vk::ImageLayout::eDepthReadOnlyOptimal : vk::ImageLayout::eDepthAttachmentOptimal};
			const vk::AccessFlags2 dst_access_flags{p_read_only ? vk::AccessFlagBits2::eDepthStencilAttachmentRead : vk::AccessFlagBits2::eDepthStencilAttachmentWrite};
			p_image->getDevice()->transitionImageLayout(p_image->getImage(), vk::ImageLayout::eShaderReadOnlyOptimal, new_layout, vk::AccessFlagBits2::eShaderRead,
														dst_access_flags, vk::PipelineStageFlagBits2::eFragmentShader,
														vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
														p_image->getSpecInfo().layerCount, p_image->getSpecInfo().mipCount, vk::ImageAspectFlagBits::eDepth);

			p_image->setCurrentImageLayout(new_layout);
		}

		auto shaderReadToTransferSrc(VKRawImage *p_image) -> void
		{
			p_image->getDevice()->transitionImageLayout(p_image->getImage(), vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eTransferSrcOptimal,
														vk::AccessFlagBits2::eShaderRead, vk::AccessFlagBits2::eTransferRead, vk::PipelineStageFlagBits2::eFragmentShader,
														vk::PipelineStageFlagBits2::eTransfer, p_image->getSpecInfo().layerCount, p_image->getSpecInfo().mipCount,
														vk::ImageAspectFlagBits::eColor);
			p_image->setCurrentImageLayout(vk::ImageLayout::eTransferSrcOptimal);
		}

		auto shaderReadToTransferDst(VKRawImage *p_image) -> void
		{
			p_image->getDevice()->transitionImageLayout(p_image->getImage(), vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eTransferDstOptimal,
														vk::AccessFlagBits2::eShaderRead, vk::AccessFlagBits2::eTransferWrite,
														vk::PipelineStageFlagBits2::eFragmentShader, vk::PipelineStageFlagBits2::eTransfer,
														p_image->getSpecInfo().layerCount, p_image->getSpecInfo().mipCount, vk::ImageAspectFlagBits::eColor);
			p_image->setCurrentImageLayout(vk::ImageLayout::eTransferDstOptimal);
		}

		auto shaderReadToGeneral(VKRawImage *p_image) -> void
		{
		}

		auto transferSrcToColourAttachment(VKRawImage *p_image) -> void
		{
		}

		auto transferSrcToDepthAttachment(VKRawImage *p_image, bool p_read_only) -> void
		{
		}

		auto transferSrcToShaderRead(VKRawImage *p_image) -> void
		{
			p_image->getDevice()->transitionImageLayout(p_image->getImage(), vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
														vk::AccessFlagBits2::eTransferRead, vk::AccessFlagBits2::eShaderRead, vk::PipelineStageFlagBits2::eTransfer,
														vk::PipelineStageFlagBits2::eFragmentShader, p_image->getSpecInfo().layerCount, p_image->getSpecInfo().mipCount,
														vk::ImageAspectFlagBits::eColor);
			p_image->setCurrentImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
		}

		auto transferSrcToTransferDst(VKRawImage *p_image) -> void
		{
			p_image->getDevice()->transitionImageLayout(p_image->getImage(), vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eTransferDstOptimal,
														vk::AccessFlagBits2::eTransferRead, vk::AccessFlagBits2::eTransferWrite, vk::PipelineStageFlagBits2::eTransfer,
														vk::PipelineStageFlagBits2::eTransfer, p_image->getSpecInfo().layerCount, p_image->getSpecInfo().mipCount,
														vk::ImageAspectFlagBits::eColor);
			p_image->setCurrentImageLayout(vk::ImageLayout::eTransferDstOptimal);
		}

		auto transferSrcToGeneral(VKRawImage *p_image) -> void
		{
		}

		auto transferDstToColourAttachment(VKRawImage *p_image) -> void
		{
		}

		auto transferDstToDepthAttachment(VKRawImage *p_image, bool p_read_only) -> void
		{
		}

		auto transferDstToShaderRead(VKRawImage *p_image) -> void
		{
			p_image->getDevice()->transitionImageLayout(p_image->getImage(), vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
														vk::AccessFlagBits2::eTransferWrite, vk::AccessFlagBits2::eShaderRead, vk::PipelineStageFlagBits2::eTransfer,
														vk::PipelineStageFlagBits2::eFragmentShader, p_image->getSpecInfo().layerCount, p_image->getSpecInfo().mipCount,
														vk::ImageAspectFlagBits::eColor);
			p_image->setCurrentImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
		}

		auto transferDstToTransferSrc(VKRawImage *p_image) -> void
		{
		}

		auto transferDstToGeneral(VKRawImage *p_image) -> void
		{
			p_image->getDevice()->transitionImageLayout(p_image->getImage(), vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eGeneral,
														vk::AccessFlagBits2::eTransferWrite, vk::AccessFlagBits2::eShaderRead | vk::AccessFlagBits2::eShaderWrite,
														vk::PipelineStageFlagBits2::eTransfer,
														vk::PipelineStageFlagBits2::eComputeShader | vk::PipelineStageFlagBits2::eFragmentShader,
														p_image->getSpecInfo().layerCount, p_image->getSpecInfo().mipCount, vk::ImageAspectFlagBits::eColor);
			p_image->setCurrentImageLayout(vk::ImageLayout::eGeneral);
		}

		auto generalToColourAttachment(VKRawImage *p_image) -> void
		{
		}

		auto generalToDepthAttachment(VKRawImage *p_image, bool p_read_only) -> void
		{
		}

		auto generalToShaderRead(VKRawImage *p_image) -> void
		{
		}

		auto generalToTransferSrc(VKRawImage *p_image) -> void
		{
		}

		auto generalToTransferDst(VKRawImage *p_image) -> void
		{
			p_image->getDevice()->transitionImageLayout(p_image->getImage(), vk::ImageLayout::eGeneral, vk::ImageLayout::eTransferDstOptimal,
														vk::AccessFlagBits2::eShaderRead | vk::AccessFlagBits2::eShaderWrite, vk::AccessFlagBits2::eTransferWrite,
														vk::PipelineStageFlagBits2::eFragmentShader | vk::PipelineStageFlagBits2::eComputeShader,
														vk::PipelineStageFlagBits2::eTransfer, p_image->getSpecInfo().layerCount, p_image->getSpecInfo().mipCount,
														vk::ImageAspectFlagBits::eColor);
			p_image->setCurrentImageLayout(vk::ImageLayout::eTransferDstOptimal);
		}

		auto undefinedToColourAttachment(VKRawImage *p_image) -> void
		{
			p_image->getDevice()->transitionImageLayout(p_image->getImage(), vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal,
														vk::AccessFlagBits2::eNone, vk::AccessFlagBits2::eColorAttachmentWrite, vk::PipelineStageFlagBits2::eTopOfPipe,
														vk::PipelineStageFlagBits2::eColorAttachmentOutput, p_image->getSpecInfo().layerCount,
														p_image->getSpecInfo().mipCount, vk::ImageAspectFlagBits::eColor);
			p_image->setCurrentImageLayout(vk::ImageLayout::eColorAttachmentOptimal);
		}

		auto undefinedToDepthAttachment(VKRawImage *p_image, bool p_read_only) -> void
		{
			const vk::ImageLayout  new_layout{p_read_only ? vk::ImageLayout::eDepthReadOnlyOptimal : vk::ImageLayout::eDepthAttachmentOptimal};
			const vk::AccessFlags2 dst_access_flags{p_read_only ? vk::AccessFlagBits2::eDepthStencilAttachmentRead : vk::AccessFlagBits2::eDepthStencilAttachmentWrite};

			p_image->getDevice()->transitionImageLayout(p_image->getImage(), vk::ImageLayout::eUndefined, new_layout, vk::AccessFlagBits2::eNone, dst_access_flags,
														vk::PipelineStageFlagBits2::eTopOfPipe,
														vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
														p_image->getSpecInfo().layerCount, p_image->getSpecInfo().mipCount, vk::ImageAspectFlagBits::eDepth);
			p_image->setCurrentImageLayout(new_layout);
		}

		auto undefinedToShaderRead(VKRawImage *p_image) -> void
		{
		}

		auto undefinedToTransferSrc(VKRawImage *p_image) -> void
		{
			p_image->getDevice()->transitionImageLayout(p_image->getImage(), vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferSrcOptimal,
														vk::AccessFlagBits2::eNone, vk::AccessFlagBits2::eTransferRead, vk::PipelineStageFlagBits2::eTopOfPipe,
														vk::PipelineStageFlagBits2::eTransfer, p_image->getSpecInfo().layerCount, p_image->getSpecInfo().mipCount,
														vk::ImageAspectFlagBits::eColor);
			p_image->setCurrentImageLayout(vk::ImageLayout::eTransferSrcOptimal);
		}

		auto undefinedToTransferDst(VKRawImage *p_image) -> void
		{
			p_image->getDevice()->transitionImageLayout(p_image->getImage(), vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
														vk::AccessFlagBits2::eNone, vk::AccessFlagBits2::eTransferWrite, vk::PipelineStageFlagBits2::eTopOfPipe,
														vk::PipelineStageFlagBits2::eTransfer, p_image->getSpecInfo().layerCount, p_image->getSpecInfo().mipCount,
														vk::ImageAspectFlagBits::eColor);
			p_image->setCurrentImageLayout(vk::ImageLayout::eTransferDstOptimal);
		}

		auto undefinedToGeneral(VKRawImage *p_image) -> void
		{
			p_image->getDevice()->transitionImageLayout(p_image->getImage(), vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral, vk::AccessFlagBits2::eNone,
														vk::AccessFlagBits2::eShaderRead | vk::AccessFlagBits2::eShaderWrite, vk::PipelineStageFlagBits2::eTopOfPipe,
														vk::PipelineStageFlagBits2::eComputeShader | vk::PipelineStageFlagBits2::eFragmentShader,
														p_image->getSpecInfo().layerCount, p_image->getSpecInfo().mipCount, vk::ImageAspectFlagBits::eColor);
			p_image->setCurrentImageLayout(vk::ImageLayout::eGeneral);
		}

		auto toColourAttachment(VKRawImage *p_image) -> void
		{
			vk::ImageLayout layout{p_image->getCurrentImageLayout()};
			switch (layout)
			{
				case vk::ImageLayout::eColorAttachmentOptimal:
					break;
				case vk::ImageLayout::eDepthAttachmentOptimal:
				case vk::ImageLayout::eDepthReadOnlyOptimal:
				{
					TST_ASSERT_MSG(false, "Cannot transition from depth attachment to colour attachment");
					break;
				}
				case vk::ImageLayout::eShaderReadOnlyOptimal:
					shaderReadToColourAttachment(p_image);
					break;
				case vk::ImageLayout::eTransferSrcOptimal:
					transferSrcToColourAttachment(p_image);
					break;
				case vk::ImageLayout::eTransferDstOptimal:
					transferDstToColourAttachment(p_image);
					break;
				case vk::ImageLayout::eGeneral:
					generalToColourAttachment(p_image);
					break;
				case vk::ImageLayout::eUndefined:
					undefinedToColourAttachment(p_image);
					break;
				default: break;
			}
		}

		auto toDepthAttachment(VKRawImage *p_image, bool p_read_only) -> void
		{
			vk::ImageLayout layout{p_image->getCurrentImageLayout()};
			switch (layout)
			{
				case vk::ImageLayout::eColorAttachmentOptimal:
				{
					TST_ASSERT_MSG(false, "Cannot transition from colour attachment to depth attachment");
					break;
				}
				case vk::ImageLayout::eDepthAttachmentOptimal:
				case vk::ImageLayout::eDepthReadOnlyOptimal:
					break;
				case vk::ImageLayout::eShaderReadOnlyOptimal:
					shaderReadToDepthAttachment(p_image, p_read_only);
					break;
				case vk::ImageLayout::eTransferSrcOptimal:
					transferSrcToDepthAttachment(p_image, p_read_only);
					break;
				case vk::ImageLayout::eTransferDstOptimal:
					transferDstToDepthAttachment(p_image, p_read_only);
					break;
				case vk::ImageLayout::eGeneral:
					generalToDepthAttachment(p_image, p_read_only);
					break;
				case vk::ImageLayout::eUndefined:
					undefinedToDepthAttachment(p_image, p_read_only);
					break;
				default: break;
			}
		}

		auto toShaderRead(VKRawImage *p_image) -> void
		{
			vk::ImageLayout layout{p_image->getCurrentImageLayout()};
			switch (layout)
			{
				case vk::ImageLayout::eColorAttachmentOptimal:
					colourAttachmentToShaderRead(p_image);
					break;
				case vk::ImageLayout::eDepthAttachmentOptimal:
					depthAttachmentToShaderRead(p_image, false);
					break;
				case vk::ImageLayout::eDepthReadOnlyOptimal:
					depthAttachmentToShaderRead(p_image, true);
					break;
				case vk::ImageLayout::eShaderReadOnlyOptimal:
					break;
				case vk::ImageLayout::eTransferSrcOptimal:
					transferSrcToShaderRead(p_image);
					break;
				case vk::ImageLayout::eTransferDstOptimal:
					transferDstToShaderRead(p_image);
					break;
				case vk::ImageLayout::eGeneral:
					generalToShaderRead(p_image);
					break;
				case vk::ImageLayout::eUndefined:
					undefinedToShaderRead(p_image);
					break;
				default: break;
			}
		}

		auto toTransferSrc(VKRawImage *p_image) -> void
		{
			vk::ImageLayout layout{p_image->getCurrentImageLayout()};
			switch (layout)
			{
				case vk::ImageLayout::eColorAttachmentOptimal:
					colourAttachmentToTransferSrc(p_image);
					break;
				case vk::ImageLayout::eDepthAttachmentOptimal:
					depthAttachmentToTransferSrc(p_image, false);
					break;
				case vk::ImageLayout::eDepthReadOnlyOptimal:
					depthAttachmentToTransferSrc(p_image, true);
					break;
				case vk::ImageLayout::eShaderReadOnlyOptimal:
					shaderReadToTransferSrc(p_image);
					break;
				case vk::ImageLayout::eTransferSrcOptimal:
					break;
				case vk::ImageLayout::eTransferDstOptimal:
					transferDstToTransferSrc(p_image);
					break;
				case vk::ImageLayout::eGeneral:
					generalToTransferSrc(p_image);
					break;
				case vk::ImageLayout::eUndefined:
					undefinedToTransferSrc(p_image);
					break;
				default: break;
			}
		}

		auto toTransferDst(VKRawImage *p_image) -> void
		{
			vk::ImageLayout layout{p_image->getCurrentImageLayout()};
			switch (layout)
			{
				case vk::ImageLayout::eColorAttachmentOptimal:
					colourAttachmentToTransferDst(p_image);
					break;
				case vk::ImageLayout::eDepthAttachmentOptimal:
					depthAttachmentToTransferDst(p_image, false);
					break;
				case vk::ImageLayout::eDepthReadOnlyOptimal:
					depthAttachmentToTransferDst(p_image, true);
					break;
				case vk::ImageLayout::eShaderReadOnlyOptimal:
					shaderReadToTransferDst(p_image);
					break;
				case vk::ImageLayout::eTransferSrcOptimal:
					transferSrcToTransferDst(p_image);
					break;
				case vk::ImageLayout::eTransferDstOptimal:
					break;
				case vk::ImageLayout::eGeneral:
					generalToTransferDst(p_image);
					break;
				case vk::ImageLayout::eUndefined:
					undefinedToTransferDst(p_image);
					break;
				default: break;
			}
		}

		auto toGeneral(VKRawImage *p_image) -> void
		{
			vk::ImageLayout layout{p_image->getCurrentImageLayout()};
			switch (layout)
			{
				case vk::ImageLayout::eColorAttachmentOptimal:
					colourAttachmentToTransferDst(p_image);
					break;
				case vk::ImageLayout::eDepthAttachmentOptimal:
					depthAttachmentToTransferDst(p_image, false);
					break;
				case vk::ImageLayout::eDepthReadOnlyOptimal:
					depthAttachmentToTransferDst(p_image, true);
					break;
				case vk::ImageLayout::eShaderReadOnlyOptimal:
					shaderReadToTransferDst(p_image);
					break;
				case vk::ImageLayout::eTransferSrcOptimal:
					transferSrcToTransferDst(p_image);
					break;
				case vk::ImageLayout::eTransferDstOptimal:
					transferDstToGeneral(p_image);
					break;
				case vk::ImageLayout::eGeneral: break;
				case vk::ImageLayout::eUndefined:
					undefinedToGeneral(p_image);
					break;
				default: break;
			}
		}
	}
}
