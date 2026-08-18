#include "toast_gpu/texture.hpp"

namespace toaster::gpu
{
	TextureManager::TextureManager(LogicalDevice &p_device, Allocator &p_allocator, ResourceDescriptorHeap &p_resource_heap) : m_device(&p_device),
																															   m_allocator(&p_allocator),
																															   m_resourceHeap(&p_resource_heap)
	{
	}

	TextureManager::~TextureManager()
	{
	}

	auto TextureManager::createTexture(const TextureDesc &p_desc) -> TextureHandle
	{
		TextureData texture_data{};
		texture_data.extent     = p_desc.extent;
		texture_data.usageFlags = p_desc.usageFlags;
		texture_data.format     = p_desc.format;
		texture_data.layout     = vk::ImageLayout::eUndefined;
		texture_data.imageType  = p_desc.imageType;
		texture_data.layerCount = p_desc.layerCount;
		texture_data.mipLevels  = p_desc.mipLevels;

		vk::ImageCreateInfo image_create_info{};
		image_create_info.imageType   = texture_data.imageType;
		image_create_info.format      = texture_data.format;
		image_create_info.extent      = texture_data.extent;
		image_create_info.mipLevels   = texture_data.mipLevels;
		image_create_info.arrayLayers = texture_data.layerCount;
		image_create_info.samples     = vk::SampleCountFlagBits::e1;
		image_create_info.tiling      = vk::ImageTiling::eOptimal;
		image_create_info.usage       = texture_data.usageFlags;
		image_create_info.sharingMode = vk::SharingMode::eExclusive;
		image_create_info.flags       = (texture_data.layerCount > 1) ? vk::ImageCreateFlagBits::eCubeCompatible : static_cast<vk::ImageCreateFlagBits>(0);

		VmaAllocationCreateFlags allocation_flags{0u};
		m_allocator->createImage(image_create_info, allocation_flags, texture_data.image, texture_data.allocation);

		return m_pool.create(texture_data);
	}

	auto TextureManager::destroyTexture(TextureHandle p_handle) -> void
	{
		// TODO: defer
		if (!m_pool.isValid(p_handle))
			TST_PERMA_ASSERT(false);

		m_pool.destroy(p_handle);
		TextureData *data{&m_pool._data[p_handle.id]};

		if (data->image && data->allocation)
		{
			vmaDestroyImage(m_allocator->getAllocator(), data->image, data->allocation);
			data->image = nullptr;
		}
		if (data->imageView)
			m_device->getDevice().destroyImageView(data->imageView);

		if (data->shaderReadHeapID != invalidImageDescriptorSlot)
			m_resourceHeap->freeImageSlot(data->shaderReadHeapID);
		if (data->storageHeapID != invalidImageDescriptorSlot)
			m_resourceHeap->freeImageSlot(data->storageHeapID);
	}

	auto TextureManager::createTextureShaderReadHeapID(TextureHandle p_handle) -> void
	{
		TextureData *data{getData(p_handle)};
		TST_PERMA_ASSERT(data);
		if (data->shaderReadHeapID != invalidImageDescriptorSlot)
			TST_PERMA_ASSERT_MSG(false, "texture already has an associated shader read heap ID");
		data->shaderReadHeapID = m_resourceHeap->allocImageSlot();
	}

	auto TextureManager::createTextureStorageHeapID(TextureHandle p_handle) -> void
	{
		TextureData *data{getData(p_handle)};
		TST_PERMA_ASSERT(data);
		if (data->storageHeapID != invalidImageDescriptorSlot)
			TST_PERMA_ASSERT_MSG(false, "texture already has an associated storage heap ID");
		data->storageHeapID = m_resourceHeap->allocImageSlot();
	}

	auto TextureManager::setTextureShaderReadHeapInfo(TextureHandle   p_handle, const vk::ImageViewCreateInfo &p_image_view_create_info,
													  vk::ImageLayout p_target_layout) -> void
	{
		TextureData *data{getData(p_handle)};
		TST_PERMA_ASSERT(data);
		if (data->shaderReadHeapID == invalidImageDescriptorSlot)
			TST_PERMA_ASSERT_MSG(false, "texture does not have an associated shader read ID");

		m_resourceHeap->setImage(data->shaderReadHeapID, p_image_view_create_info, p_target_layout, vk::DescriptorType::eSampledImage);
	}

	auto TextureManager::setTextureStorageHeapInfo(TextureHandle   p_handle, const vk::ImageViewCreateInfo &p_image_view_create_info,
												   vk::ImageLayout p_target_layout) -> void
	{
		TextureData *data{getData(p_handle)};
		TST_PERMA_ASSERT(data);
		if (data->storageHeapID == invalidImageDescriptorSlot)
			TST_PERMA_ASSERT_MSG(false, "texture does not have an associated storage ID");

		m_resourceHeap->setImage(data->storageHeapID, p_image_view_create_info, p_target_layout, vk::DescriptorType::eStorageImage);
	}

	auto TextureManager::isValid(TextureHandle p_handle) const -> bool
	{
		return m_pool.isValid(p_handle);
	}

	auto TextureManager::getData(TextureHandle p_handle) -> TextureData *
	{
		return m_pool.getData(p_handle);
	}
}
