#include "toast_render/image.hpp"

#include "toast_render/render_context.hpp"

namespace toaster::render
{
	Image::Image(RenderContext &p_render_ctx, const ImageSpecInfo &p_spec_info) : m_renderCtx(&p_render_ctx), m_specInfo(p_spec_info)
	{
		vk::ImageUsageFlags usage_flags{vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled};
		if (m_specInfo.storage)
			usage_flags |= vk::ImageUsageFlagBits::eStorage;

		gpu::ImageSpecInfo image_spec_info{};
		image_spec_info.size   = m_specInfo.size;
		image_spec_info.format = m_specInfo.format;
		image_spec_info.usage  = usage_flags;
		if (m_specInfo.generateMipmaps)
			image_spec_info.mipCount = static_cast<uint32>(std::floor(std::log2(std::max(m_specInfo.size.x, m_specInfo.size.y)))) + 1u;
		image_spec_info.layerCount = m_specInfo.layerCount;
		image_spec_info.hostAccess = m_specInfo.hostAccess;

		// You will only use this constructor if you are going to set the data
		m_image = m_renderCtx->createGPURef<gpu::RawImage>(image_spec_info);

		if (m_specInfo.storage)
			m_storageHeapID = m_renderCtx->getDescriptorHeap()->allocImage(*m_image, true);
		m_shaderReadHeapID = m_renderCtx->getDescriptorHeap()->allocImage(*m_image, false);
	}

	Image::Image(RenderContext &p_render_ctx, const ImageSpecInfo &p_spec_info, const Buffer &p_data) : m_renderCtx(&p_render_ctx), m_specInfo(p_spec_info)
	{
		vk::ImageUsageFlags usage_flags{vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eSampled};
		if (m_specInfo.storage)
			usage_flags |= vk::ImageUsageFlagBits::eStorage;

		gpu::ImageSpecInfo image_spec_info{};
		image_spec_info.size   = m_specInfo.size;
		image_spec_info.format = m_specInfo.format;
		image_spec_info.usage  = usage_flags;
		if (m_specInfo.generateMipmaps)
			image_spec_info.mipCount = static_cast<uint32>(std::floor(std::log2(std::max(m_specInfo.size.x, m_specInfo.size.y)))) + 1u;
		image_spec_info.layerCount = m_specInfo.layerCount;
		image_spec_info.hostAccess = m_specInfo.hostAccess;
		m_image                    = m_renderCtx->createGPURef<gpu::RawImage>(image_spec_info);

		gpu::util::toTransferDst(m_image.get());
		m_image->setData(p_data);

		// if (m_specInfo.generateMipmaps)
		// m_renderCtx->getLogicalDevice()->generateMipmaps(m_image->getImage(), {m_specInfo.size.x, m_specInfo.size.y, 1u}, image_spec_info.mipCount);

		// m_image->setCurrentImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal); // Generate mips leaves the image in the eShaderReadOnlyOptimal layout

		if (m_specInfo.storage)
			m_storageHeapID = m_renderCtx->getDescriptorHeap()->allocImage(*m_image, true);
		m_shaderReadHeapID = m_renderCtx->getDescriptorHeap()->allocImage(*m_image, false);
	}

	Image::Image(RenderContext &p_render_ctx, const gpu::RawImageHandle &p_raw_image) : m_renderCtx(&p_render_ctx), m_image(p_raw_image)
	{
		m_specInfo.size       = m_image->getSpecInfo().size;
		m_specInfo.layerCount = m_image->getSpecInfo().layerCount;
		m_specInfo.format     = m_image->getSpecInfo().format;
		m_specInfo.storage    = !!(m_image->getSpecInfo().usage & vk::ImageUsageFlagBits::eStorage);
		m_specInfo.hostAccess = m_image->getSpecInfo().hostAccess;

		if (m_specInfo.storage)
			m_storageHeapID = m_renderCtx->getDescriptorHeap()->allocImage(*m_image, true);
		m_shaderReadHeapID = m_renderCtx->getDescriptorHeap()->allocImage(*m_image, false);
	}

	Image::~Image()
	{
		if (m_renderCtx)
		{
			if (m_specInfo.storage)
			{
				m_renderCtx->getDescriptorHeap()->freeImage(m_storageHeapID);

				for (const auto mip_id: m_perMipStorageHeapIDs | std::views::values)
					m_renderCtx->getDescriptorHeap()->freeImage(mip_id);
			}
			m_renderCtx->getDescriptorHeap()->freeImage(m_shaderReadHeapID);
			for (const auto mip_id: m_perMipShaderReadHeapIDs | std::views::values)
				m_renderCtx->getDescriptorHeap()->freeImage(mip_id);
		}
	}

	auto Image::setData(const Buffer &p_data) -> void
	{
		gpu::util::toTransferDst(m_image.get());
		m_image->setData(p_data);
		gpu::util::transferDstToShaderRead(m_image.get());
	}

	auto Image::generateMipmaps() -> void
	{
		vk::ImageLayout prev_layout{m_image->getCurrentImageLayout()};
		if (m_specInfo.generateMipmaps)
			m_renderCtx->getGPUContext()->generateMipmaps(m_image->getImage(), {m_specInfo.size.x, m_specInfo.size.y, 1u}, m_image->getSpecInfo().mipCount);

		m_image->setCurrentImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal); // Generate mips leaves the image in the eShaderReadOnlyOptimal layout
		gpu::util::transitionImageLayout(m_image.get(), vk::ImageLayout::eShaderReadOnlyOptimal, prev_layout);
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

	auto Image::resize(ImageSize p_size) -> void
	{
		m_image->resize(p_size);

		if (m_specInfo.storage)
			m_renderCtx->getDescriptorHeap()->setImage(m_storageHeapID, *m_image);
		m_renderCtx->getDescriptorHeap()->setImage(m_shaderReadHeapID, *m_image);
	}

	auto Image::toStorageOptimal() -> void
	{
		gpu::util::toGeneral(m_image);
	}

	auto Image::toShaderReadOptimal() -> void
	{
		gpu::util::toShaderRead(m_image);
	}

	auto Image::getStorageHeapID() const -> gpu::DescriptorSlot
	{
		return m_storageHeapID;
	}

	auto Image::getShaderReadHeapID() const -> gpu::DescriptorSlot
	{
		return m_shaderReadHeapID;
	}

	auto Image::getAlignedStorageHeapID() const -> gpu::DescriptorSlot
	{
		return m_storageHeapID + (m_renderCtx->getDescriptorHeap()->getImageOffset() / m_renderCtx->getDescriptorHeap()->getImageDescriptorSize());
	}

	auto Image::getAlignedShaderReadHeapID() const -> gpu::DescriptorSlot
	{
		return m_shaderReadHeapID + (m_renderCtx->getDescriptorHeap()->getImageOffset() / m_renderCtx->getDescriptorHeap()->getImageDescriptorSize());
	}

	auto Image::createMipHeapID(uint32 p_level) -> void
	{
		if (m_specInfo.storage)
			m_perMipStorageHeapIDs[p_level] = m_renderCtx->getDescriptorHeap()->allocImage(*m_image, true, p_level);

		m_perMipShaderReadHeapIDs[p_level] = m_renderCtx->getDescriptorHeap()->allocImage(*m_image, false, p_level);
	}

	auto Image::getMipStorageHeapID(uint32 p_level) const -> gpu::DescriptorSlot
	{
		return m_perMipStorageHeapIDs.at(p_level);
	}

	auto Image::getMipShaderReadHeapID(uint32 p_level) const -> gpu::DescriptorSlot
	{
		return m_perMipShaderReadHeapIDs.at(p_level);
	}

	auto Image::getMipAlignedStorageHeapID(uint32 p_level) const -> gpu::DescriptorSlot
	{
		return m_perMipStorageHeapIDs.at(p_level) + (m_renderCtx->getDescriptorHeap()->getImageOffset() / m_renderCtx->getDescriptorHeap()->getImageDescriptorSize());
	}

	auto Image::getMipAlignedShaderReadHeapID(uint32 p_level) const -> gpu::DescriptorSlot
	{
		return m_perMipShaderReadHeapIDs.at(p_level) + (m_renderCtx->getDescriptorHeap()->getImageOffset() / m_renderCtx->getDescriptorHeap()->getImageDescriptorSize());
	}
}
