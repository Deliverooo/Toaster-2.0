#include "toast_gpu/device.hpp"

#include "toast_gpu/command_list.hpp"

#include <d3d12.h>

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

	Device::Device(const DeviceDesc p_desc)
	{
		FunctionDispatcher::initBaseFunctions();
		InstanceDesc instance_desc{};
		instance_desc.enableValidationLayers = p_desc.enableDebugInfo;
		instance_desc.requiredExtensions     = {};
		if (p_desc.usingSwapchain)
		{
			instance_desc.requiredExtensions.emplace(vk::KHRSurfaceExtensionName);
			instance_desc.requiredExtensions.emplace(vk::KHRWin32SurfaceExtensionName);
		}
		instance_desc.applicationName = "Orbo's Exile";
		m_instance                    = makeUnique<Instance>(instance_desc);
		FunctionDispatcher::initInstanceFunctions(m_instance->getInstance());

		PhysicalDeviceDesc physical_device_desc{};
		physical_device_desc.requiredExtensions = {
			vk::EXTDescriptorHeapExtensionName,
			vk::EXTShaderObjectExtensionName,
			vk::KHRMaintenance1ExtensionName,
			vk::KHRShaderUntypedPointersExtensionName,
			vk::KHRMaintenance9ExtensionName
		};
		if (p_desc.usingSwapchain)
			physical_device_desc.requiredExtensions.emplace(vk::KHRSwapchainExtensionName);

		m_physicalDevice = makeUnique<PhysicalDevice>(*m_instance, physical_device_desc);

		vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan12Features, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceVulkan14Features,
			vk::PhysicalDeviceShaderObjectFeaturesEXT, vk::PhysicalDeviceDescriptorHeapFeaturesEXT, vk::PhysicalDeviceShaderUntypedPointersFeaturesKHR,
			vk::PhysicalDeviceMaintenance9FeaturesKHR> feature_chain{{}, {}, {}, {}, {}, {}, {}, {}};

		feature_chain.get<vk::PhysicalDeviceFeatures2>().features.samplerAnisotropy                        = true;
		feature_chain.get<vk::PhysicalDeviceFeatures2>().features.sampleRateShading                        = true;
		feature_chain.get<vk::PhysicalDeviceFeatures2>().features.fillModeNonSolid                         = true;
		feature_chain.get<vk::PhysicalDeviceFeatures2>().features.fragmentStoresAndAtomics                 = true;
		feature_chain.get<vk::PhysicalDeviceFeatures2>().features.shaderInt16                              = true;
		feature_chain.get<vk::PhysicalDeviceFeatures2>().features.shaderInt64                              = true;
		feature_chain.get<vk::PhysicalDeviceFeatures2>().features.vertexPipelineStoresAndAtomics           = true;
		feature_chain.get<vk::PhysicalDeviceFeatures2>().features.multiDrawIndirect                        = true;
		feature_chain.get<vk::PhysicalDeviceVulkan12Features>().timelineSemaphore                          = true;
		feature_chain.get<vk::PhysicalDeviceVulkan12Features>().bufferDeviceAddress                        = true;
		feature_chain.get<vk::PhysicalDeviceVulkan12Features>().runtimeDescriptorArray                     = true;
		feature_chain.get<vk::PhysicalDeviceVulkan12Features>().shaderInt8                                 = true;
		feature_chain.get<vk::PhysicalDeviceVulkan12Features>().storagePushConstant8                       = true;
		feature_chain.get<vk::PhysicalDeviceVulkan12Features>().uniformAndStorageBuffer8BitAccess          = true;
		feature_chain.get<vk::PhysicalDeviceVulkan12Features>().storageBuffer8BitAccess                    = true;
		feature_chain.get<vk::PhysicalDeviceVulkan12Features>().shaderStorageBufferArrayNonUniformIndexing = true;
		feature_chain.get<vk::PhysicalDeviceVulkan12Features>().shaderUniformBufferArrayNonUniformIndexing = true;
		feature_chain.get<vk::PhysicalDeviceVulkan12Features>().shaderSampledImageArrayNonUniformIndexing  = true;
		feature_chain.get<vk::PhysicalDeviceVulkan12Features>().shaderUniformBufferArrayNonUniformIndexing = true;
		feature_chain.get<vk::PhysicalDeviceVulkan12Features>().shaderStorageImageArrayNonUniformIndexing  = true;
		feature_chain.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering                           = true;
		feature_chain.get<vk::PhysicalDeviceVulkan13Features>().synchronization2                           = true;
		feature_chain.get<vk::PhysicalDeviceVulkan13Features>().maintenance4                               = true;
		feature_chain.get<vk::PhysicalDeviceVulkan14Features>().indexTypeUint8                             = true;
		feature_chain.get<vk::PhysicalDeviceVulkan14Features>().maintenance6                               = true;
		feature_chain.get<vk::PhysicalDeviceShaderObjectFeaturesEXT>().shaderObject                        = true;
		feature_chain.get<vk::PhysicalDeviceDescriptorHeapFeaturesEXT>().descriptorHeap                    = true;
		feature_chain.get<vk::PhysicalDeviceShaderUntypedPointersFeaturesKHR>().shaderUntypedPointers      = true;
		feature_chain.get<vk::PhysicalDeviceMaintenance9FeaturesKHR>().maintenance9                        = true;

		LogicalDeviceDesc logical_device_desc{};
		logical_device_desc.pNextDeviceFeatures = feature_chain.get<vk::PhysicalDeviceFeatures2>();
		logical_device_desc.enabledExtensions   = physical_device_desc.requiredExtensions;
		m_device                                = makeUnique<LogicalDevice>(*m_physicalDevice, logical_device_desc);
		FunctionDispatcher::initDeviceFunctions(m_device->getDevice());

		m_allocator = makeUnique<Allocator>(*m_instance, *m_physicalDevice, *m_device);

		m_resourceHeap = makeUnique<ResourceDescriptorHeap>(*m_device, *m_physicalDevice, *m_allocator, p_desc.maxBufferDescriptors, p_desc.maxImageDescriptors);
		m_samplerHeap  = makeUnique<SamplerDescriptorHeap>(*m_device, *m_physicalDevice, *m_allocator, p_desc.maxSamplerDescriptors);

		m_deletionQueue = makeUnique<DeletionQueue>(p_desc.numDeletionQueues);

		m_bufferPool.setUserData(this);
		m_bufferPool.setDestroyCallback(+[](void *p_user_data, BufferHandle p_handle) -> void
		{
			auto       ts{static_cast<Device *>(p_user_data)};
			BufferData buffer_data{ts->m_bufferPool._data[p_handle.id]};

			ts->submitDeletion([allocator = ts->m_allocator->getAllocator(), buffer_data]() mutable noexcept -> void
			{
				TST_ASSERT(buffer_data.buffer && buffer_data.allocation);
				vmaDestroyBuffer(allocator, buffer_data.buffer, buffer_data.allocation);
			});
		});

		m_texturePool.setUserData(this);
		m_texturePool.setDestroyCallback(+[](void *p_user_data, TextureHandle p_handle) -> void
		{
			auto ts{static_cast<Device *>(p_user_data)};

			TextureData texture_data{ts->m_texturePool._data[p_handle.id]};

			ts->submitDeletion([ts, texture_data]() mutable noexcept -> void
			{
				if (texture_data.imageView)
					ts->m_device->getDevice().destroyImageView(texture_data.imageView);
				if (texture_data.image && texture_data.allocation)
					vmaDestroyImage(ts->m_allocator->getAllocator(), texture_data.image, texture_data.allocation);

				if (texture_data.shaderReadHeapID != invalidImageDescriptorSlot)
					ts->m_resourceHeap->freeImageSlot(texture_data.shaderReadHeapID);
				if (texture_data.storageHeapID != invalidImageDescriptorSlot)
					ts->m_resourceHeap->freeImageSlot(texture_data.storageHeapID);
			});
		});

		m_samplerPool.setUserData(this);
		m_samplerPool.setDestroyCallback(+[](void *p_user_data, SamplerHandle p_handle) -> void
		{
			auto        ts{static_cast<Device *>(p_user_data)};
			SamplerData sampler_data{ts->m_samplerPool._data[p_handle.id]};

			ts->submitDeletion([ts, sampler_data]() mutable noexcept -> void
			{
				ts->m_samplerHeap->freeSamplerSlot(sampler_data.heapID);
			});
		});

		m_shaderPool.setUserData(this);
		m_shaderPool.setDestroyCallback(+[](void *p_user_data, ShaderHandle p_handle) -> void
		{
			auto       ts{static_cast<Device *>(p_user_data)};
			ShaderData shader_data{ts->m_shaderPool._data[p_handle.id]};

			ts->submitDeletion([ts, shader_data]() mutable noexcept -> void
			{
				if (shader_data.shader)
					ts->m_device->getDevice().destroyShaderEXT(shader_data.shader, nullptr, FunctionDispatcher::get());
			});
		});
	}

	Device::~Device()
	{
		// Delete all remaining shaders
		for (uint32 i{0u}; i < m_shaderPool.getSize(); ++i)
		{
			if (m_shaderPool._alive[i])
			{
				ShaderData shader_data{m_shaderPool._data[i]};

				m_deletionQueue->submit([this, shader_data]() mutable noexcept -> void
				{
					if (shader_data.shader)
						m_device->getDevice().destroyShaderEXT(shader_data.shader, nullptr, FunctionDispatcher::get());
				});
			}
		}

		// Delete all remaining samplers
		for (uint32 i{0u}; i < m_samplerPool.getSize(); ++i)
		{
			if (m_samplerPool._alive[i])
			{
				SamplerData sampler_data{m_samplerPool._data[i]};

				m_deletionQueue->submit([this, sampler_data]() mutable noexcept -> void
				{
					m_samplerHeap->freeSamplerSlot(sampler_data.heapID);
				});
			}
		}

		// Delete all remaining textures
		for (uint32 i{0u}; i < m_texturePool.getSize(); ++i)
		{
			if (m_texturePool._alive[i])
			{
				TextureData texture_data{m_texturePool._data[i]};

				m_deletionQueue->submit([this, texture_data]() mutable noexcept -> void
				{
					if (texture_data.imageView)
						m_device->getDevice().destroyImageView(texture_data.imageView);
					if (texture_data.image && texture_data.allocation)
						vmaDestroyImage(m_allocator->getAllocator(), texture_data.image, texture_data.allocation);

					if (texture_data.shaderReadHeapID != invalidImageDescriptorSlot)
						m_resourceHeap->freeImageSlot(texture_data.shaderReadHeapID);
					if (texture_data.storageHeapID != invalidImageDescriptorSlot)
						m_resourceHeap->freeImageSlot(texture_data.storageHeapID);
				});
			}
		}

		// Delete all remaining buffers
		for (uint32 i{0u}; i < m_bufferPool.getSize(); ++i)
		{
			if (m_bufferPool._alive[i])
			{
				BufferData buffer_data{m_bufferPool._data[i]}; // Copy
				TST_PERMA_ASSERT(buffer_data.buffer && buffer_data.allocation);
				m_deletionQueue->submit([this, buffer_data]() mutable noexcept -> void // The allocator must be destroyed after the deletion queue is executed
				{
					if (buffer_data.buffer && buffer_data.allocation)
						vmaDestroyBuffer(m_allocator->getAllocator(), buffer_data.buffer, buffer_data.allocation);
				});
			}
		}

		m_deletionQueue->executeAll();
		m_deletionQueue.reset();

		m_samplerHeap.reset();
		m_resourceHeap.reset();

		m_allocator.reset();

		m_device.reset();
		m_physicalDevice.reset();
		m_instance.reset();
	}

	auto Device::performGarbageCollection(uint32 p_queue_index) -> void
	{
		m_deletionQueue->execute();
		m_deletionQueue->setQueueIndex(p_queue_index);
	}

	auto Device::flushDeletionQueue() -> void
	{
		m_deletionQueue->executeAll();
	}

	auto Device::updateResourceDescriptorWrites() -> void
	{
		m_resourceHeap->writeDescriptors();
	}

	auto Device::updateSamplerDescriptorWrites() -> void
	{
		m_samplerHeap->writeDescriptors();
	}

	auto Device::createBuffer(const BufferDesc &p_desc) -> BufferHandle
	{
		BufferData buffer_data{};
		buffer_data.usageFlags       = p_desc.usageFlags;
		buffer_data.size             = p_desc.size;
		buffer_data.memoryProperties = p_desc.memoryProperties;

		if (buffer_data.memoryProperties == EMemoryProperties::eDeviceLocal)
			m_allocator->createBuffer(buffer_data.size, buffer_data.usageFlags, 0u, buffer_data.buffer, buffer_data.allocation);
		else if (buffer_data.memoryProperties == EMemoryProperties::eHostVisibleCoherent)
		{
			m_allocator->createBuffer(buffer_data.size, buffer_data.usageFlags, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
									  buffer_data.buffer, buffer_data.allocation, &buffer_data.mapped);
		}
		else
			TST_PERMA_ASSERT_MSG(false, "What is dis?");

		if (p_desc.usageFlags & vk::BufferUsageFlagBits::eShaderDeviceAddress)
			buffer_data.address = getBufferAddress(buffer_data.buffer);

		return m_bufferPool.create(buffer_data);
	}

	auto Device::uploadBufferData(BufferHandle p_handle, const void *p_data, uint64 p_size, uint64 p_offset) -> void
	{
		BufferData *dst_buffer{m_bufferPool.getData(p_handle)};
		TST_ASSERT(dst_buffer && dst_buffer->buffer && dst_buffer->allocation && dst_buffer->memoryProperties == EMemoryProperties::eHostVisibleCoherent);

		std::memcpy(reinterpret_cast<void *>(reinterpret_cast<uint64>(dst_buffer->mapped) + p_offset), p_data, p_size);
	}

	auto Device::createTexture(const TextureDesc &p_desc) -> TextureHandle
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
			m_allocator->createImage(image_create_info, 0u, texture_data.image, texture_data.allocation);
		}

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

		if (isRenderTarget(p_desc.usageFlags))
		{
			texture_data.imageView = m_device->getDevice().createImageView(image_view_create_info);
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

		return m_texturePool.create(texture_data);
	}

	auto Device::getTextureShaderReadHeapSlot(TextureHandle p_handle) const -> uint32
	{
		const TextureData *data{getTextureData(p_handle)};
		return m_resourceHeap->getImageAbsoluteHeapSlot(data->shaderReadHeapID);
	}

	auto Device::getTextureStorageHeapSlot(TextureHandle p_handle) const -> uint32
	{
		const TextureData *data{getTextureData(p_handle)};
		return m_resourceHeap->getImageAbsoluteHeapSlot(data->storageHeapID);
	}

	auto Device::createSampler(const SamplerDesc &p_desc) -> SamplerHandle
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
		sampler_create_info.maxAnisotropy = m_physicalDevice->getProperties().properties.limits.maxSamplerAnisotropy;
		sampler_create_info.compareEnable = false;
		sampler_create_info.compareOp = vk::CompareOp::eAlways;
		sampler_create_info.minLod = 0.0f;
		sampler_create_info.maxLod = vk::LodClampNone;
		sampler_create_info.borderColor = vk::BorderColor::eFloatOpaqueWhite;
		sampler_create_info.unnormalizedCoordinates = false;

		sampler_data.heapID = m_samplerHeap->allocSamplerSlot();
		m_samplerHeap->setSampler(sampler_data.heapID, sampler_create_info);

		return m_samplerPool.create(sampler_data);
	}

	auto Device::createShader(const ShaderDesc &p_desc) -> ShaderHandle
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

		shader_data.shader = m_device->getDevice().createShaderEXT(shader_create_info, nullptr, FunctionDispatcher::get()).value;
		TST_ASSERT(shader_data.shader);

		return m_shaderPool.create(shader_data);
	}

	auto Device::createCommandList() -> CommandList
	{
		vk::CommandBufferAllocateInfo cmd_alloc_info{};
		cmd_alloc_info.commandPool        = m_device->getGraphicsCommandPool();
		cmd_alloc_info.commandBufferCount = 1u;
		cmd_alloc_info.level              = vk::CommandBufferLevel::ePrimary;

		vk::CommandBuffer cmd{m_device->getDevice().allocateCommandBuffers(cmd_alloc_info).front()};

		return {*this, cmd};
	}

	auto Device::createCommandLists(const uint32 p_count) -> std::vector<CommandList>
	{
		vk::CommandBufferAllocateInfo cmd_alloc_info{};
		cmd_alloc_info.commandPool        = m_device->getGraphicsCommandPool();
		cmd_alloc_info.commandBufferCount = p_count;
		cmd_alloc_info.level              = vk::CommandBufferLevel::ePrimary;

		const std::vector<vk::CommandBuffer> command_buffers{m_device->getDevice().allocateCommandBuffers(cmd_alloc_info)};

		std::vector<CommandList> out_lists;
		for (const auto cmd: command_buffers)
			out_lists.emplace_back(CommandList{*this, cmd});
		return out_lists;
	}

	auto Device::executeCommandLists(const InitialiserList<const CommandList> &            p_command_lists,
									 const InitialiserList<const vk::SemaphoreSubmitInfo> &p_wait_semaphores,
									 const InitialiserList<const vk::SemaphoreSubmitInfo> &p_signal_semaphores, vk::Fence p_signal_fence) const -> void
	{
		std::vector<vk::CommandBufferSubmitInfo> command_buffer_infos;
		for (const auto cmd: p_command_lists)
			command_buffer_infos.emplace_back(vk::CommandBufferSubmitInfo{cmd.getCommandBuffer()});

		vk::SubmitInfo2 submit_info{};
		submit_info.setCommandBufferInfos(command_buffer_infos);
		submit_info.setWaitSemaphoreInfos(p_wait_semaphores);
		submit_info.setSignalSemaphoreInfos(p_signal_semaphores);

		m_device->getGraphicsQueue().submit2(submit_info, p_signal_fence);
	}

	auto Device::createTimelineSemaphore(uint64 p_initial_value) const -> vk::Semaphore
	{
		vk::SemaphoreTypeCreateInfo timeline_semaphore_create_info{};
		timeline_semaphore_create_info.initialValue  = p_initial_value;
		timeline_semaphore_create_info.semaphoreType = vk::SemaphoreType::eTimeline;

		vk::SemaphoreCreateInfo create_info{};
		create_info.pNext = &timeline_semaphore_create_info;

		return m_device->getDevice().createSemaphore(create_info);
	}

	auto Device::waitForTimelineSemaphores(const InitialiserList<const vk::Semaphore> &p_semaphores, const InitialiserList<const uint64> &p_target_values) const -> void
	{
		vk::SemaphoreWaitInfo semaphore_wait_info{};
		semaphore_wait_info.setSemaphores(p_semaphores);
		semaphore_wait_info.setValues(p_target_values);
		if (m_device->getDevice().waitSemaphores(semaphore_wait_info, INFINITE) != vk::Result::eSuccess)
			TST_PERMA_ASSERT_MSG(false, "Failed to wait for timeline semaphores");
	}

	auto Device::getVulkanShaderStages(ShaderStageFlags p_stages) -> vk::ShaderStageFlags
	{
		vk::ShaderStageFlags out_flags{0u};
		if (p_stages & EShaderStage::eVertex)
			out_flags |= vk::ShaderStageFlagBits::eVertex;
		if (p_stages & EShaderStage::ePixel)
			out_flags |= vk::ShaderStageFlagBits::eFragment;
		if (p_stages & EShaderStage::eCompute)
			out_flags |= vk::ShaderStageFlagBits::eCompute;
		if (p_stages & EShaderStage::eGeometry)
			out_flags |= vk::ShaderStageFlagBits::eGeometry;
		if (p_stages & EShaderStage::eTessellationControl)
			out_flags |= vk::ShaderStageFlagBits::eTessellationControl;
		if (p_stages & EShaderStage::eTessellationEvaluation)
			out_flags |= vk::ShaderStageFlagBits::eTessellationEvaluation;
		if (p_stages & EShaderStage::eMesh)
			out_flags |= vk::ShaderStageFlagBits::eMeshEXT;
		if (p_stages & EShaderStage::eTask)
			out_flags |= vk::ShaderStageFlagBits::eTaskEXT;

		return out_flags;
	}
}
