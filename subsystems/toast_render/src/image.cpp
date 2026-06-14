#include "toast_render/image.hpp"

#include "toast_render/render_context.hpp"

namespace toaster::render
{
	Image::Image(RenderContext &p_render_ctx, const ImageSpecInfo &p_spec_info) : m_renderCtx(&p_render_ctx), m_specInfo(p_spec_info)
	{
		m_specInfo.usageFlags |= vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
		gpu::ImageSpecInfo image_spec_info{};
		image_spec_info.size       = m_specInfo.size;
		image_spec_info.format     = m_specInfo.format;
		image_spec_info.usage      = m_specInfo.usageFlags;
		image_spec_info.layerCount = m_specInfo.layerCount;

		// You will only use this constructor if you are going to set the data
		m_image = m_renderCtx->createGPURef<gpu::RawImage>(image_spec_info);

		m_heapID = m_renderCtx->getDescriptorHeap()->allocImage(*m_image);
	}

	Image::Image(RenderContext &p_render_ctx, const ImageSpecInfo &p_spec_info, const Buffer &p_data) : m_renderCtx(&p_render_ctx), m_specInfo(p_spec_info)
	{
		m_specInfo.usageFlags |= vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eSampled;

		gpu::ImageSpecInfo image_spec_info{};
		image_spec_info.size       = m_specInfo.size;
		image_spec_info.format     = m_specInfo.format;
		image_spec_info.usage      = m_specInfo.usageFlags;
		image_spec_info.layerCount = m_specInfo.layerCount;
		m_image                    = m_renderCtx->createGPURef<gpu::RawImage>(image_spec_info);

		gpu::util::toTransferDst(m_image.get());
		m_image->setData(p_data);

		m_renderCtx->getLogicalDevice()->generateMipmaps(m_image->getImage(), {m_specInfo.size.x, m_specInfo.size.y, 1u}, 1);
		m_image->setCurrentImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal); // Generate mips leaves the image in the eShaderReadOnlyOptimal layout

		m_heapID = m_renderCtx->getDescriptorHeap()->allocImage(*m_image);
	}

	Image::~Image()
	{
		if (m_renderCtx)
			m_renderCtx->getDescriptorHeap()->freeImage(m_heapID);
	}

	auto Image::setData(const Buffer &p_data) -> void
	{
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

	auto Image::getImage() -> gpu::RawImageHandle &
	{
		return m_image;
	}

	auto Image::resize(tsm::uint2 p_size) -> void
	{
		m_image->resize(p_size);
		m_renderCtx->getDescriptorHeap()->setImage(m_heapID, *m_image);
	}

	auto Image::getHeapID() const -> gpu::DescriptorSlot
	{
		return m_heapID;
	}

	auto Image::getAlignedHeapID() const -> gpu::DescriptorSlot
	{
		return m_heapID + (m_renderCtx->getDescriptorHeap()->getImageOffset() / m_renderCtx->getDescriptorHeap()->getImageDescriptorSize());
	}
}
