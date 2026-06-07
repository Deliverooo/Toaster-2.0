#include "toast_render/image.hpp"

#include "toast_render/render_context.hpp"

namespace toaster::render
{
	Image::Image(RenderContext &p_render_ctx, const ImageSpecInfo &p_spec_info) : m_renderCtx(&p_render_ctx), m_specInfo(p_spec_info)
	{
		gpu::ImageSpecInfo image_spec_info{};
		image_spec_info.size   = m_specInfo.size;
		image_spec_info.format = m_specInfo.format;
		image_spec_info.usage  = m_specInfo.usageFlags | vk::ImageUsageFlagBits::eTransferDst; // You will only use this constructor if you are going to set the data
		m_image                = m_renderCtx->createGPURef<gpu::RawImage>(image_spec_info);

		m_heapID = m_renderCtx->getDescriptorHeap()->allocImage(*m_image);
	}

	Image::Image(RenderContext &p_render_ctx, const io::filesystem::Path &p_path) : m_renderCtx(&p_render_ctx)
	{
		m_imageData = gpu::util::loadTextureIntoBuffer(p_path, m_specInfo.format, m_specInfo.size.x, m_specInfo.size.y);
		if (!m_imageData)
			TST_PERMA_ASSERT(false);

		gpu::ImageSpecInfo image_spec_info{};
		image_spec_info.size   = m_specInfo.size;
		image_spec_info.format = m_specInfo.format;
		image_spec_info.usage  = m_specInfo.usageFlags | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc;
		m_image                = m_renderCtx->createGPURef<gpu::RawImage>(image_spec_info);

		gpu::util::toTransferDst(m_image.get());
		m_image->setData(m_imageData);

		m_renderCtx->getLogicalDevice()->generateMipmaps(m_image->getImage(), {m_specInfo.size.x, m_specInfo.size.y, 1u}, 1);
		m_image->setCurrentImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal); // Generate mips leaves the image in the eShaderReadOnlyOptimal layout

		m_heapID = m_renderCtx->getDescriptorHeap()->allocImage(*m_image);
	}

	Image::Image(RenderContext &p_render_ctx, const ImageSpecInfo &p_spec_info, const Buffer &p_data) : m_renderCtx(&p_render_ctx), m_specInfo(p_spec_info),
																										m_imageData(p_data)
	{
		gpu::ImageSpecInfo image_spec_info{};
		image_spec_info.size   = m_specInfo.size;
		image_spec_info.format = m_specInfo.format;
		image_spec_info.usage  = m_specInfo.usageFlags | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc;
		m_image                = m_renderCtx->createGPURef<gpu::RawImage>(image_spec_info);

		gpu::util::toTransferDst(m_image.get());
		m_image->setData(m_imageData);

		m_renderCtx->getLogicalDevice()->generateMipmaps(m_image->getImage(), {m_specInfo.size.x, m_specInfo.size.y, 1u}, 1);
		m_image->setCurrentImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal); // Generate mips leaves the image in the eShaderReadOnlyOptimal layout

		m_heapID = m_renderCtx->getDescriptorHeap()->allocImage(*m_image);
	}

	Image::~Image()
	{
		m_imageData.release();
		if (m_renderCtx)
			m_renderCtx->getDescriptorHeap()->freeImage(m_heapID);
	}

	auto Image::setData(const Buffer &p_data) -> void
	{
		m_imageData.release();
		m_imageData = p_data;

		gpu::util::toTransferDst(m_image.get());
		m_image->setData(p_data);
		gpu::util::transferDstToShaderRead(m_image.get());
	}

	auto Image::getSpecInfo() const -> const ImageSpecInfo &
	{
		return m_specInfo;
	}

	auto Image::getImage() const -> const gpu::RawImageHandle &
	{
		return m_image;
	}

	auto Image::getHeapID() const -> gpu::DescriptorSlot
	{
		return m_heapID;
	}

	auto Image::getAlignedHeapID() const -> gpu::DescriptorSlot
	{
		return m_heapID + (m_renderCtx->getDescriptorHeap()->getImageOffset() / m_renderCtx->getDescriptorHeap()->getHeapProperties().imageDescriptorSize);
	}
}
