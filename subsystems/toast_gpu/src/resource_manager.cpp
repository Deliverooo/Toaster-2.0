#include "toast_gpu/resource_manager.hpp"

namespace toaster::gpu
{
	ResourceManager::ResourceManager(Device &p_device, const ResourceManagerDesc &p_desc) : m_device(&p_device)
	{
		m_resourceHeap = makeUnique<ResourceDescriptorHeap>(m_device->getDevice(), m_device->getPhysicalDevice(), m_device->getAllocator(), p_desc.maxBufferDescriptors,
															p_desc.maxImageDescriptors);
		m_samplerHeap = makeUnique<SamplerDescriptorHeap>(m_device->getDevice(), m_device->getPhysicalDevice(), m_device->getAllocator(), p_desc.maxSamplerDescriptors);
	}

	ResourceManager::~ResourceManager()
	{
		m_device->waitIdle();

		m_bufferPool.purgeAll([this](BufferData &p_data) -> void { _destroyBuffer(p_data); });
		m_texturePool.purgeAll([this](TextureData &p_data) -> void { _destroyTexture(p_data); });
		m_samplerPool.purgeAll([this](SamplerData &p_data) -> void { _destroySampler(p_data); });
		m_shaderPool.purgeAll([this](ShaderData &p_data) -> void { _destroyShader(p_data); });

		m_samplerHeap.reset();
		m_resourceHeap.reset();
	}

	auto ResourceManager::updateResourceDescriptorWrites() -> void
	{
		m_resourceHeap->writeDescriptors();
	}

	auto ResourceManager::updateSamplerDescriptorWrites() -> void
	{
		m_samplerHeap->writeDescriptors();
	}

	auto ResourceManager::createBuffer(const BufferDesc &p_desc) -> BufferHandle
	{
		BufferData buffer_data{};
		buffer_data.usageFlags       = p_desc.usageFlags;
		buffer_data.size             = p_desc.size;
		buffer_data.memoryProperties = p_desc.memoryProperties;

		if (buffer_data.memoryProperties == EMemoryProperties::eDeviceLocal)
			m_device->getAllocator().createBuffer(buffer_data.size, buffer_data.usageFlags, 0u, buffer_data.buffer, buffer_data.allocation);
		else if (buffer_data.memoryProperties == EMemoryProperties::eHostVisibleCoherent)
		{
			m_device->getAllocator().createBuffer(buffer_data.size, buffer_data.usageFlags,
												  VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, buffer_data.buffer,
												  buffer_data.allocation, &buffer_data.mapped);
		}
		else
			TST_PERMA_ASSERT_MSG(false, "What is dis?");

		if (p_desc.usageFlags & vk::BufferUsageFlagBits::eShaderDeviceAddress)
			buffer_data.address = m_device->getDevice().getDevice().getBufferAddress(buffer_data.buffer);

		return m_bufferPool.emplace(buffer_data);
	}

	auto ResourceManager::createTexture(const TextureDesc &p_desc) -> TextureHandle
	{
		TextureData texture_data{};
		texture_data.extent     = p_desc.extent;
		texture_data.usageFlags = p_desc.usageFlags;
		texture_data.layerCount = p_desc.layerCount;
		texture_data.mipLevels  = p_desc.mipLevels;
		texture_data.format     = p_desc.format;
		texture_data.layout     = vk::ImageLayout::eUndefined;
		texture_data.type       = p_desc.type;

		if (p_desc.existingImage)
		{
			texture_data.image      = p_desc.existingImage;
			texture_data.allocation = nullptr;
		}
		else
		{
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
			m_device->getAllocator().createImage(image_create_info, 0u, texture_data.image, texture_data.allocation);
		}

		vk::ImageViewCreateInfo image_view_create_info{};
		image_view_create_info.image      = texture_data.image;
		image_view_create_info.viewType   = (p_desc.layerCount == 6u) ? vk::ImageViewType::eCube : getVulkanImageViewType(p_desc.type);
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

		if (isRenderTarget(p_desc.usageFlags))
		{
			texture_data.imageView = m_device->getDevice().getDevice().createImageView(image_view_create_info);
		}

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

		return m_texturePool.emplace(texture_data);
	}

	auto ResourceManager::createSampler(const SamplerDesc &p_desc) -> SamplerHandle
	{
		constexpr auto getAddressMode{
			+[](ESamplerAddressMode p_address_mode) -> vk::SamplerAddressMode
			{
				switch (p_address_mode)
				{
					case ESamplerAddressMode::eRepeat: return vk::SamplerAddressMode::eRepeat;
					case ESamplerAddressMode::eMirroredRepeat: return vk::SamplerAddressMode::eMirroredRepeat;
					case ESamplerAddressMode::eClampToEdge: return vk::SamplerAddressMode::eClampToEdge;
					case ESamplerAddressMode::eClampToBorder: return vk::SamplerAddressMode::eClampToBorder;
				}
				return vk::SamplerAddressMode::eRepeat;
			}
		};
		SamplerData sampler_data{};
		sampler_data.minFilter  = p_desc.minFilter;
		sampler_data.magFilter  = p_desc.magFilter;
		sampler_data.mipmapMode = p_desc.mipmapMode;

		vk::SamplerCreateInfo sampler_create_info{};
		sampler_create_info.minFilter = sampler_data.minFilter == EFilter::eLinear ? vk::Filter::eLinear : vk::Filter::eNearest;
		sampler_create_info.magFilter = sampler_data.magFilter == EFilter::eLinear ? vk::Filter::eLinear : vk::Filter::eNearest;
		sampler_create_info.mipmapMode = sampler_data.mipmapMode == ESamplerMipmapMode::eLinear ? vk::SamplerMipmapMode::eLinear : vk::SamplerMipmapMode::eNearest;
		sampler_create_info.addressModeU = getAddressMode(sampler_data.addressModeU);
		sampler_create_info.addressModeV = getAddressMode(sampler_data.addressModeV);
		sampler_create_info.addressModeW = getAddressMode(sampler_data.addressModeW);
		sampler_create_info.mipLodBias = 0.0f;
		sampler_create_info.anisotropyEnable = true;
		sampler_create_info.maxAnisotropy = m_device->getPhysicalDevice().getProperties().properties.limits.maxSamplerAnisotropy;
		sampler_create_info.compareEnable = false;
		sampler_create_info.compareOp = vk::CompareOp::eAlways;
		sampler_create_info.minLod = 0.0f;
		sampler_create_info.maxLod = vk::LodClampNone;
		sampler_create_info.borderColor = vk::BorderColor::eFloatOpaqueWhite;
		sampler_create_info.unnormalizedCoordinates = false;

		sampler_data.heapID = m_samplerHeap->allocSamplerSlot();
		m_samplerHeap->setSampler(sampler_data.heapID, sampler_create_info);

		return m_samplerPool.emplace(sampler_data);
	}

	auto ResourceManager::createShader(const ShaderDesc &p_desc) -> ShaderHandle
	{
		ShaderData shader_data{};
		shader_data.stage     = p_desc.stage;
		shader_data.nextStage = p_desc.nextStage;

		vk::ShaderCreateInfoEXT shader_create_info{};
		shader_create_info.flags     = vk::ShaderCreateFlagBitsEXT::eDescriptorHeap;
		shader_create_info.stage     = (vk::ShaderStageFlags::BitsType) ((vk::ShaderStageFlags::MaskType) getVulkanShaderStages(shader_data.stage));
		shader_create_info.nextStage = getVulkanShaderStages(shader_data.nextStage);
		shader_create_info.codeType  = vk::ShaderCodeTypeEXT::eSpirv;
		shader_create_info.codeSize  = p_desc.codeSize;
		shader_create_info.pCode     = p_desc.code;
		shader_create_info.pName     = "main";

		shader_data.shader = m_device->getDevice().getDevice().createShaderEXT(shader_create_info, nullptr, FunctionDispatcher::get()).value;
		TST_ASSERT(shader_data.shader);

		return m_shaderPool.emplace(shader_data);
	}

	auto ResourceManager::destroyBuffer(BufferHandle p_handle) -> void
	{
		m_bufferPool.destroy(p_handle, m_currentTimelineValue);
	}

	auto ResourceManager::destroyTexture(TextureHandle p_handle) -> void
	{
		m_texturePool.destroy(p_handle, m_currentTimelineValue);
	}

	auto ResourceManager::destroySampler(SamplerHandle p_handle) -> void
	{
		m_samplerPool.destroy(p_handle, m_currentTimelineValue);
	}

	auto ResourceManager::destroyShader(ShaderHandle p_handle) -> void
	{
		m_shaderPool.destroy(p_handle, m_currentTimelineValue);
	}

	auto ResourceManager::performGarbageCollection(uint64 p_current_timeline_value) -> void
	{
		m_bufferPool.cleanupDeletions(p_current_timeline_value, [this](BufferData &p_data) -> void { _destroyBuffer(p_data); });
		m_texturePool.cleanupDeletions(p_current_timeline_value, [this](TextureData &p_data) -> void { _destroyTexture(p_data); });
		m_samplerPool.cleanupDeletions(p_current_timeline_value, [this](SamplerData &p_data) -> void { _destroySampler(p_data); });
		m_shaderPool.cleanupDeletions(p_current_timeline_value, [this](ShaderData &p_data) -> void { _destroyShader(p_data); });
	}

	auto ResourceManager::uploadBufferData(BufferHandle p_handle, const void *p_data, uint64 p_size, uint64 p_offset) -> void
	{
		TST_ASSERT(isBufferValid(p_handle));
		BufferData *dst_buffer{getBufferData(p_handle)};
		TST_ASSERT(dst_buffer && dst_buffer->buffer && dst_buffer->allocation && dst_buffer->memoryProperties == gpu::EMemoryProperties::eHostVisibleCoherent);
		std::memcpy(reinterpret_cast<void *>(reinterpret_cast<uint64>(dst_buffer->mapped) + p_offset), p_data, p_size);
	}

	auto ResourceManager::getTextureShaderReadHeapSlot(TextureHandle p_handle) const -> uint32
	{
		const TextureData* texture_data{getTextureData(p_handle)};
		TST_ASSERT(texture_data);
		return m_resourceHeap->getImageAbsoluteHeapSlot(texture_data->shaderReadHeapID);
	}

	auto ResourceManager::getTextureStorageHeapSlot(TextureHandle p_handle) const -> uint32
	{
		const TextureData* texture_data{getTextureData(p_handle)};
		TST_ASSERT(texture_data);
		return m_resourceHeap->getImageAbsoluteHeapSlot(texture_data->storageHeapID);
	}

	auto ResourceManager::_destroyBuffer(BufferData &p_data) -> void
	{
		TST_ASSERT(p_data.buffer && p_data.allocation);
		vmaDestroyBuffer(m_device->getAllocator().getAllocator(), p_data.buffer, p_data.allocation);
	}

	auto ResourceManager::_destroyTexture(TextureData &p_data) -> void
	{
		if (p_data.imageView)
			m_device->getDevice().getDevice().destroyImageView(p_data.imageView);
		if (p_data.image && p_data.allocation)
			vmaDestroyImage(m_device->getAllocator().getAllocator(), p_data.image, p_data.allocation);

		if (p_data.shaderReadHeapID != invalidImageDescriptorSlot)
			m_resourceHeap->freeImageSlot(p_data.shaderReadHeapID);
		if (p_data.storageHeapID != invalidImageDescriptorSlot)
			m_resourceHeap->freeImageSlot(p_data.storageHeapID);
	}

	auto ResourceManager::_destroySampler(SamplerData &p_data) -> void
	{
		m_samplerHeap->freeSamplerSlot(p_data.heapID);
	}

	auto ResourceManager::_destroyShader(ShaderData &p_data) -> void
	{
		if (p_data.shader)
			m_device->getDevice().getDevice().destroyShaderEXT(p_data.shader, nullptr, FunctionDispatcher::get());
	}
}
