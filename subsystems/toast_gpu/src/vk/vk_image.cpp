#include "toast_gpu/vk/vk_image.hpp"
#include "toast_gpu/vk/vk_gpu_context.hpp"

namespace toaster::gpu
{
	VKImage::VKImage(VKGPUContext &p_gpu_ctx, const ImageSpecInfo &p_spec_info) : m_gpuCtx(&p_gpu_ctx), m_specInfo(p_spec_info)
	{
		vk::ImageUsageFlags usage_flags{vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled};
		if (m_specInfo.storage)
			usage_flags |= vk::ImageUsageFlagBits::eStorage;

		RawImageSpecInfo image_spec_info{};
		image_spec_info.size   = m_specInfo.size;
		image_spec_info.format = m_specInfo.format;
		image_spec_info.usage  = usage_flags;
		if (m_specInfo.mipLevels == ImageSpecInfo::eComputeMipLevels)
			image_spec_info.mipCount = static_cast<uint32>(std::floor(std::log2(std::max(m_specInfo.size.x, m_specInfo.size.y)))) + 1u;
		else if (m_specInfo.mipLevels != 0u)
			image_spec_info.mipCount = m_specInfo.mipLevels;
		image_spec_info.layerCount = m_specInfo.layerCount;
		image_spec_info.hostAccess = m_specInfo.hostAccess;

		// You will only use this constructor if you are going to set the data
		m_image = makeReference<VKRawImage>(*m_gpuCtx, image_spec_info);

		if (m_specInfo.storage)
			m_storageDescriptorSlot = m_gpuCtx->getDescriptorHeap()->allocImage(*m_image, true);
		m_shaderDescriptorSlot = m_gpuCtx->getDescriptorHeap()->allocImage(*m_image, false);
	}

	VKImage::VKImage(VKGPUContext &p_gpu_ctx, const ImageSpecInfo &p_spec_info, const toaster::Buffer &p_data) : m_gpuCtx(&p_gpu_ctx), m_specInfo(p_spec_info)
	{
		vk::ImageUsageFlags usage_flags{vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eSampled};
		if (m_specInfo.storage)
			usage_flags |= vk::ImageUsageFlagBits::eStorage;

		RawImageSpecInfo image_spec_info{};
		image_spec_info.size   = m_specInfo.size;
		image_spec_info.format = m_specInfo.format;
		image_spec_info.usage  = usage_flags;
		if (m_specInfo.mipLevels == ImageSpecInfo::eComputeMipLevels)
			image_spec_info.mipCount = static_cast<uint32>(std::floor(std::log2(std::max(m_specInfo.size.x, m_specInfo.size.y)))) + 1u;
		else if (m_specInfo.mipLevels != 0u)
			image_spec_info.mipCount = m_specInfo.mipLevels;
		image_spec_info.layerCount = m_specInfo.layerCount;
		image_spec_info.hostAccess = m_specInfo.hostAccess;
		m_image                    = makeReference<VKRawImage>(*m_gpuCtx, image_spec_info);

		util::toTransferDst(m_image.get());
		m_image->setData(p_data);

		if (m_specInfo.storage)
			m_storageDescriptorSlot = m_gpuCtx->getDescriptorHeap()->allocImage(*m_image, true);
		m_shaderDescriptorSlot = m_gpuCtx->getDescriptorHeap()->allocImage(*m_image, false);
	}

	VKImage::VKImage(VKGPUContext &p_gpu_ctx, const RawImageHandle &p_raw_image) : m_gpuCtx(&p_gpu_ctx), m_image(p_raw_image)
	{
		m_specInfo.size       = m_image->getSpecInfo().size;
		m_specInfo.layerCount = m_image->getSpecInfo().layerCount;
		m_specInfo.format     = m_image->getSpecInfo().format;
		m_specInfo.storage    = !!(m_image->getSpecInfo().usage & vk::ImageUsageFlagBits::eStorage);
		m_specInfo.hostAccess = m_image->getSpecInfo().hostAccess;

		if (m_specInfo.storage)
			m_storageDescriptorSlot = m_gpuCtx->getDescriptorHeap()->allocImage(*m_image, true);
		m_shaderDescriptorSlot = m_gpuCtx->getDescriptorHeap()->allocImage(*m_image, false);
	}

	VKImage::~VKImage()
	{
		if (m_gpuCtx)
		{
			if (m_specInfo.storage)
			{
				m_gpuCtx->getDescriptorHeap()->freeImage(m_storageDescriptorSlot);

				for (const auto mip_id: m_perMipStorageDescriptorSlots | std::views::values)
					m_gpuCtx->getDescriptorHeap()->freeImage(mip_id);
			}
			m_gpuCtx->getDescriptorHeap()->freeImage(m_shaderDescriptorSlot);
			for (const auto mip_id: m_perMipShaderReadDescriptorSlots | std::views::values)
				m_gpuCtx->getDescriptorHeap()->freeImage(mip_id);
		}
	}

	auto VKImage::setData(const toaster::Buffer &p_data) -> void
	{
		util::toTransferDst(m_image.get());
		m_image->setData(p_data);
		util::transferDstToShaderRead(m_image.get());
	}

	auto VKImage::generateMipmaps() -> void
	{
		vk::ImageLayout prev_layout{m_image->getCurrentImageLayout()};
		if (m_specInfo.mipLevels == ImageSpecInfo::eComputeMipLevels)
			m_gpuCtx->generateMipmaps(m_image->getImage(), {m_specInfo.size.x, m_specInfo.size.y, 1u}, m_image->getSpecInfo().mipCount);

		m_image->setCurrentImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal); // Generate mips leaves the image in the eShaderReadOnlyOptimal layout
		util::transitionImageLayout(m_image.get(), vk::ImageLayout::eShaderReadOnlyOptimal, prev_layout);
	}

	auto VKImage::getSpecInfo() const -> const ImageSpecInfo &
	{
		return m_specInfo;
	}

	auto VKImage::getImage() const -> const RawImageHandle &
	{
		return m_image;
	}

	auto VKImage::getImage() -> RawImageHandle &
	{
		return m_image;
	}

	auto VKImage::resize(ImageSize p_size) -> void
	{
		m_image->resize(p_size);

		if (m_specInfo.storage)
			m_gpuCtx->getDescriptorHeap()->setImage(m_storageDescriptorSlot, *m_image);
		m_gpuCtx->getDescriptorHeap()->setImage(m_shaderDescriptorSlot, *m_image);
	}

	auto VKImage::toStorageOptimal() -> void
	{
		util::toGeneral(m_image);
	}

	auto VKImage::toShaderReadOptimal() -> void
	{
		util::toShaderRead(m_image);
	}

	auto VKImage::getStorageDescriptorSlot() const -> DescriptorSlot
	{
		return m_storageDescriptorSlot;
	}

	auto VKImage::getShaderReadDescriptorSlot() const -> DescriptorSlot
	{
		return m_shaderDescriptorSlot;
	}

	auto VKImage::getStorageHeapID() const -> DescriptorSlot
	{
		return m_storageDescriptorSlot + (m_gpuCtx->getDescriptorHeap()->getImageOffset() / m_gpuCtx->getDescriptorHeap()->getImageDescriptorSize());
	}

	auto VKImage::getShaderReadHeapID() const -> DescriptorSlot
	{
		return m_shaderDescriptorSlot + (m_gpuCtx->getDescriptorHeap()->getImageOffset() / m_gpuCtx->getDescriptorHeap()->getImageDescriptorSize());
	}

	auto VKImage::createMipDescriptorSlot(uint32 p_level) -> void
	{
		if (m_specInfo.storage)
			m_perMipStorageDescriptorSlots[p_level] = m_gpuCtx->getDescriptorHeap()->allocImage(*m_image, true, p_level);

		m_perMipShaderReadDescriptorSlots[p_level] = m_gpuCtx->getDescriptorHeap()->allocImage(*m_image, false, p_level);
	}

	auto VKImage::getMipStorageDescriptorSlot(uint32 p_level) const -> DescriptorSlot
	{
		return m_perMipStorageDescriptorSlots.at(p_level);
	}

	auto VKImage::getMipShaderReadDescriptorSlot(uint32 p_level) const -> DescriptorSlot
	{
		return m_perMipShaderReadDescriptorSlots.at(p_level);
	}

	auto VKImage::getMipStorageHeapID(uint32 p_level) const -> DescriptorSlot
	{
		return m_perMipStorageDescriptorSlots.at(p_level) + (m_gpuCtx->getDescriptorHeap()->getImageOffset() / m_gpuCtx->getDescriptorHeap()->getImageDescriptorSize());
	}

	auto VKImage::getMipShaderReadHeapID(uint32 p_level) const -> DescriptorSlot
	{
		return m_perMipShaderReadDescriptorSlots.at(p_level) + (m_gpuCtx->getDescriptorHeap()->getImageOffset() / m_gpuCtx->getDescriptorHeap()->
																getImageDescriptorSize());
	}
}
