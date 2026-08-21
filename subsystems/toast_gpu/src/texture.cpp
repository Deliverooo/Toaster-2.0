#include "toast_gpu/texture.hpp"

namespace toaster::gpu
{
	auto getImageViewType(vk::ImageType p_type) -> vk::ImageViewType
	{
		switch (p_type)
		{
			case vk::ImageType::e1D: return vk::ImageViewType::e1D;
			case vk::ImageType::e2D: return vk::ImageViewType::e2D;
			case vk::ImageType::e3D: return vk::ImageViewType::e3D;
		}
		return vk::ImageViewType::e2D;
	}

	TextureManager::TextureManager(LogicalDevice &p_device, Allocator &p_allocator, ResourceDescriptorHeap &p_resource_heap, BufferManager &p_buffer_manager,
								   void *         p_user_data, const DestroyCallback &p_destroy_callback) : m_device(&p_device), m_allocator(&p_allocator),
																											m_resourceHeap(&p_resource_heap),
																											m_bufferManager(&p_buffer_manager),
																											m_userData(p_user_data), m_destroyCallback(p_destroy_callback)
	{
		m_pool.setUserData(this);
		m_pool.setDestroyCallback(+[](void *p_user_data, TextureHandle p_handle) -> void
		{
			auto ts{static_cast<TextureManager *>(p_user_data)};

			TextureData *texture_data{&ts->m_pool._data[p_handle.id]};

			if (ts->m_destroyCallback)
				ts->m_destroyCallback(ts->m_userData, p_handle);
			else
				ts->destroyData(texture_data);
		});
	}

	TextureManager::~TextureManager()
	{
		// For safety...
		for (uint32 i{0u}; i < m_pool.getSize(); ++i)
		{
			if (m_pool._alive[i])
				destroyData(&m_pool._data[i]);
		}
	}

	auto TextureManager::createTexture(const TextureDesc &p_desc) -> TextureHandle
	{
		TextureData texture_data{};
		texture_data.extent           = p_desc.extent;
		texture_data.usageFlags       = p_desc.usageFlags;
		texture_data.layerCount       = p_desc.layerCount;
		texture_data.mipLevels        = p_desc.mipLevels;
		texture_data.format           = p_desc.format;
		texture_data.layout           = vk::ImageLayout::eUndefined;
		texture_data.type             = p_desc.type;
		texture_data.memoryProperties = p_desc.memoryProperties;

		VmaAllocationCreateFlags allocation_create_flags{
			(texture_data.memoryProperties == EMemoryProperties::eHostVisibleCoherent) ? VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT : 0u
		};

		vk::ImageCreateInfo image_create_info{};
		image_create_info.imageType     = texture_data.type;
		image_create_info.format        = texture_data.format;
		image_create_info.extent        = texture_data.extent;
		image_create_info.mipLevels     = texture_data.mipLevels;
		image_create_info.arrayLayers   = texture_data.layerCount;
		image_create_info.samples       = vk::SampleCountFlagBits::e1;
		image_create_info.tiling        = vk::ImageTiling::eOptimal;
		image_create_info.usage         = texture_data.usageFlags;
		image_create_info.sharingMode   = vk::SharingMode::eExclusive;
		image_create_info.initialLayout = vk::ImageLayout::eUndefined;
		m_allocator->createImage(image_create_info, allocation_create_flags, texture_data.image, texture_data.allocation);

		if (texture_data.memoryProperties == EMemoryProperties::eHostVisibleCoherent)
			vmaMapMemory(m_allocator->getAllocator(), texture_data.allocation, &texture_data.mapped);

		vk::ImageViewCreateInfo image_view_create_info{};
		image_view_create_info.image      = texture_data.image;
		image_view_create_info.viewType   = (p_desc.layerCount == 6u) ? vk::ImageViewType::eCube : getImageViewType(p_desc.type);
		image_view_create_info.components = vk::ComponentMapping{
			vk::ComponentSwizzle::eIdentity,
			vk::ComponentSwizzle::eIdentity,
			vk::ComponentSwizzle::eIdentity,
			vk::ComponentSwizzle::eIdentity
		};
		image_view_create_info.subresourceRange = vk::ImageSubresourceRange{
			getImageAspectMask(texture_data.format),
			0u,
			texture_data.mipLevels,
			0u,
			texture_data.layerCount
		};
		image_view_create_info.format = texture_data.format;

		if (p_desc.usageFlags & vk::ImageUsageFlagBits::eSampled && p_desc.createDescriptors)
		{
			texture_data.shaderReadHeapID = m_resourceHeap->allocImageSlot();
			m_resourceHeap->setImage(texture_data.shaderReadHeapID, image_view_create_info, vk::ImageLayout::eShaderReadOnlyOptimal, vk::DescriptorType::eSampledImage);
		}
		else if (p_desc.usageFlags & vk::ImageUsageFlagBits::eStorage && p_desc.createDescriptors)
		{
			texture_data.storageHeapID = m_resourceHeap->allocImageSlot();
			m_resourceHeap->setImage(texture_data.storageHeapID, image_view_create_info, vk::ImageLayout::eGeneral, vk::DescriptorType::eStorageImage);
		}

		return m_pool.create(texture_data);
	}

	auto TextureManager::createSharedTexture(const TextureDesc &p_desc) -> SharedHandle<TextureTag, TextureData>
	{
		return {createTexture(p_desc), &m_pool};
	}

	auto TextureManager::destroyData(TextureData *p_data) -> void
	{
		// Unmap the buffer's memory
		if (p_data->mapped && p_data->memoryProperties == EMemoryProperties::eHostVisibleCoherent)
			vmaUnmapMemory(m_allocator->getAllocator(), p_data->allocation);

		if (p_data->imageView)
			m_device->getDevice().destroyImageView(p_data->imageView);
		if (p_data->image && p_data->allocation)
		{
			vmaDestroyImage(m_allocator->getAllocator(), p_data->image, p_data->allocation);
			p_data->image      = nullptr;
			p_data->allocation = nullptr;
		}

		if (p_data->shaderReadHeapID != invalidImageDescriptorSlot)
			m_resourceHeap->freeImageSlot(p_data->shaderReadHeapID);
		if (p_data->storageHeapID != invalidImageDescriptorSlot)
			m_resourceHeap->freeImageSlot(p_data->storageHeapID);
	}

	auto TextureManager::getTextureShaderReadHeapSlot(TextureHandle p_handle) const -> DescriptorSlot
	{
		const TextureData *data{getData(p_handle)};
		return data->shaderReadHeapID + (m_resourceHeap->getImageSegmentOffset() / m_resourceHeap->getImageDescriptorSize());
	}

	auto TextureManager::getTextureStorageHeapSlot(TextureHandle p_handle) const -> DescriptorSlot
	{
		const TextureData *data{getData(p_handle)};
		return data->storageHeapID + (m_resourceHeap->getImageSegmentOffset() / m_resourceHeap->getImageDescriptorSize());
	}

	auto TextureManager::transitionTextureLayout(TextureHandle p_handle, vk::CommandBuffer p_cmd, vk::ImageLayout p_dst_layout) -> void
	{
		TextureData *data{getData(p_handle)};
		_transitionTextureLayout(data, p_cmd, p_dst_layout);
	}

	auto TextureManager::setTextureData(TextureHandle p_handle, vk::CommandBuffer p_cmd, const void *p_data, uint64 p_size, uint64 p_offset) -> void
	{
		TextureData *dst_texture{getData(p_handle)};
		TST_PERMA_ASSERT(dst_texture && dst_texture->image && dst_texture->allocation);

		if (dst_texture->memoryProperties == EMemoryProperties::eHostVisibleCoherent)
		{
			std::memcpy(reinterpret_cast<void *>(reinterpret_cast<uint64>(dst_texture->mapped) + p_offset), p_data, p_size);
			return;
		}

		BufferDesc staging_buffer_desc{};
		staging_buffer_desc.size             = p_size;
		staging_buffer_desc.usageFlags       = vk::BufferUsageFlagBits::eTransferSrc;
		staging_buffer_desc.memoryProperties = EMemoryProperties::eHostVisibleCoherent;

		auto staging_buffer{m_bufferManager->createBuffer(staging_buffer_desc)};
		m_bufferManager->uploadDirect(staging_buffer, p_data, p_size, p_offset);

		dst_texture = getData(p_handle); // FIX: Due to vector reallocation when creating the staging buffer, the dst one becomes invalid

		_transitionTextureLayout(dst_texture, p_cmd, vk::ImageLayout::eTransferDstOptimal);

		vk::BufferImageCopy2 buffer_image_copy{};
		buffer_image_copy.bufferOffset      = 0;
		buffer_image_copy.bufferRowLength   = 0;
		buffer_image_copy.bufferImageHeight = 0;
		buffer_image_copy.imageOffset       = vk::Offset3D{0, 0, 0};
		buffer_image_copy.imageExtent       = dst_texture->extent;
		buffer_image_copy.imageSubresource  = {getImageAspectMask(dst_texture->format), 0, 0, dst_texture->layerCount};

		vk::CopyBufferToImageInfo2 copy_buffer_to_image_info{};
		copy_buffer_to_image_info.srcBuffer      = m_bufferManager->getBufferBuffer(staging_buffer);
		copy_buffer_to_image_info.dstImage       = dst_texture->image;
		copy_buffer_to_image_info.dstImageLayout = vk::ImageLayout::eTransferDstOptimal;
		copy_buffer_to_image_info.setRegions(buffer_image_copy);
		p_cmd.copyBufferToImage2(copy_buffer_to_image_info);

		m_bufferManager->destroyBuffer(staging_buffer); // Defer
	}

	auto TextureManager::isValid(TextureHandle p_handle) const -> bool
	{
		return m_pool.isValid(p_handle);
	}

	auto TextureManager::getData(TextureHandle p_handle) const -> const TextureData *
	{
		return m_pool.getData(p_handle);
	}

	auto TextureManager::getData(TextureHandle p_handle) -> TextureData *
	{
		return m_pool.getData(p_handle);
	}

	auto TextureManager::_transitionTextureLayout(TextureData *p_data, vk::CommandBuffer p_cmd, vk::ImageLayout p_dst_layout) -> void
	{
		TST_PERMA_ASSERT(p_dst_layout != vk::ImageLayout::eUndefined);
		if (p_data->layout == p_dst_layout) // If the image is already in the specified layout, there is no need to transition.
			return;

		vk::ImageMemoryBarrier2 image_memory_barrier{};
		image_memory_barrier.image     = p_data->image;
		image_memory_barrier.oldLayout = p_data->layout;
		image_memory_barrier.newLayout = p_dst_layout;

		getImageAccessFlagsAndStageMask(p_data->layout, image_memory_barrier.srcAccessMask, image_memory_barrier.srcStageMask);
		getImageAccessFlagsAndStageMask(p_dst_layout, image_memory_barrier.dstAccessMask, image_memory_barrier.dstStageMask);

		image_memory_barrier.subresourceRange = vk::ImageSubresourceRange{getImageAspectMask(p_data->format), 0u, p_data->mipLevels, 0u, p_data->layerCount};

		vk::DependencyInfo dependency_info{};
		dependency_info.setImageMemoryBarriers(image_memory_barrier);

		p_cmd.pipelineBarrier2(dependency_info);

		p_data->layout = p_dst_layout;
	}
}
