#include "toast_gpu/vk/vk_descriptor_set_manager.hpp"
#include "toast_gpu/vk/vk_logical_device.hpp"

#include "toast_lib/map.hpp"

namespace toaster::gpu
{
	VKDescriptorSetManager::VKDescriptorSetManager(VKLogicalDevice *p_device, const DescriptorSetManagerSpecInfo &p_spec_info) : m_device(p_device),
																																 m_specInfo(p_spec_info)
	{
		TextureSpecInfo texture_spec_info{};
		texture_spec_info.size         = {1u};
		texture_spec_info.format       = vk::Format::eR8G8B8A8Unorm;
		texture_spec_info.generateMips = false;
		uint32 texture_data{0xFFFFFFFF};
		m_whiteTexture = make_reference<VKTexture2D>(m_device, texture_spec_info, &texture_data, sizeof(uint32));

		uint32 texture_3d_data[6]{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
		m_whiteTexture3D = make_reference<VKTexture3D>(m_device, texture_spec_info, Buffer{texture_3d_data, sizeof(uint32) * 6});

		const auto &descriptor_sets{m_specInfo.shader->getReflectedShaderDescriptorSets()};
		m_writeDescriptorMap.resize(m_device->getSpecInfo().maxFramesInFlight);

		for (uint32 set{m_specInfo.startSet}; set <= m_specInfo.endSet; ++set)
		{
			if (set >= descriptor_sets.size())
				break;

			const auto &descriptor_set{descriptor_sets[set]};
			for (auto &[name, write_descriptor]: descriptor_set.writeDescriptorSets)
			{
				uint32 binding{write_descriptor.dstBinding};

				DescriptorDeclaration &descriptor_declaration{m_descriptorDeclarations[name]};
				descriptor_declaration.name      = name;
				descriptor_declaration.set       = set;
				descriptor_declaration.binding   = binding;
				descriptor_declaration.arraySize = write_descriptor.descriptorCount;
				// This will be the final type of the descriptor unless it is an image sampler, see _getDescriptorImageSamplerType()
				descriptor_declaration.type = getDescriptorType(write_descriptor.descriptorType);

				DescriptorResource &descriptor_resource{m_descriptorResources[set][binding]};
				descriptor_resource.resources.resize(write_descriptor.descriptorCount);
				descriptor_resource.type = getResourceType(write_descriptor.descriptorType);

				// For samplers, they are different types depending on their dimension
				if (descriptor_set.imageSamplers.contains(binding))
				{
					auto &sampler{descriptor_set.imageSamplers.at(binding)};
					descriptor_declaration.type = getDescriptorImageSamplerType(write_descriptor.descriptorType, sampler.dimension);

					if (sampler.dimension == reflection::EImageDimension::e3D || sampler.dimension == reflection::EImageDimension::eCube)
						descriptor_resource.type = EGPUResourceType::eTexture3D;
				}
				else if (descriptor_set.storageImages.contains(binding))
				{
					auto &storage_image{descriptor_set.storageImages.at(binding)};
					descriptor_declaration.type = getDescriptorImageSamplerType(write_descriptor.descriptorType, storage_image.dimension);

					if (storage_image.dimension == reflection::EImageDimension::e3D || storage_image.dimension == reflection::EImageDimension::eCube)
						descriptor_resource.type = EGPUResourceType::eTexture3D;
				}

				// LOG_ERROR("Type: {}", descriptorTypeToString(descriptor_declaration.type));
				TST_ASSERT(descriptor_declaration.type != EDescriptorType::eUnknown);

				// Create default resources so you can actually set the resource descriptor during rendering
				if (descriptor_declaration.type == EDescriptorType::eSampler2D)
				{
					for (uint32 i{0u}; i < descriptor_resource.resources.size(); ++i)
						descriptor_resource.resources[i] = m_whiteTexture.as<IGPUResource>();
				}
				else if (descriptor_declaration.type == EDescriptorType::eSampler3D || descriptor_declaration.type == EDescriptorType::eImage3D)
				{
					// I actually don't think I even support 3D samplers yet...
					for (uint32 i{0u}; i < descriptor_resource.resources.size(); ++i)
						descriptor_resource.resources[i] = m_whiteTexture3D.as<IGPUResource>();
				}

				for (uint32 frame_index{0u}; frame_index < m_device->getSpecInfo().maxFramesInFlight; ++frame_index)
					m_writeDescriptorMap[frame_index][set][binding] = {write_descriptor, std::vector<void *>{write_descriptor.descriptorCount}};
			}
		}
	}

	auto VKDescriptorSetManager::bakeDescriptors() -> void
	{
		constexpr std::array descriptor_pool_sizes{
			vk::DescriptorPoolSize{vk::DescriptorType::eUniformBuffer, 100u},
			vk::DescriptorPoolSize{vk::DescriptorType::eStorageBuffer, 100u},
			vk::DescriptorPoolSize{vk::DescriptorType::eCombinedImageSampler, 100u},
			vk::DescriptorPoolSize{vk::DescriptorType::eSampledImage, 100u},
			vk::DescriptorPoolSize{vk::DescriptorType::eStorageImage, 100u}
		};

		vk::DescriptorPoolCreateInfo descriptor_pool_create_info{};
		descriptor_pool_create_info.poolSizeCount = descriptor_pool_sizes.size();
		descriptor_pool_create_info.pPoolSizes    = descriptor_pool_sizes.data();
		descriptor_pool_create_info.maxSets       = 10u * m_device->getSpecInfo().maxFramesInFlight;
		descriptor_pool_create_info.flags         = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;

		m_descriptorPool = {m_device->getVulkanLogicalDevice(), descriptor_pool_create_info};

		if (m_descriptorSets.empty())
			for (uint32 i{0u}; i < m_device->getSpecInfo().maxFramesInFlight; ++i)
				m_descriptorSets.emplace_back();

		for (auto &descriptor_set: m_descriptorSets)
			descriptor_set.clear();

		for (const auto &[set, resources]: m_descriptorResources)
		{
			for (uint32 frame_index{0u}; frame_index < m_device->getSpecInfo().maxFramesInFlight; ++frame_index)
			{
				const vk::raii::DescriptorSetLayout &descriptor_set_layout{m_specInfo.shader->getDescriptorSetLayout(set)};
				vk::DescriptorSetAllocateInfo        descriptor_set_allocate_info{};
				descriptor_set_allocate_info.descriptorPool     = m_descriptorPool;
				descriptor_set_allocate_info.descriptorSetCount = 1;
				descriptor_set_allocate_info.pSetLayouts        = &*descriptor_set_layout;

				auto &descriptor_set{
					m_descriptorSets[frame_index].emplace_back(std::move(m_device->getVulkanLogicalDevice().allocateDescriptorSets(descriptor_set_allocate_info).front()))
				};

				#ifndef NDEBUG
				m_device->setDebugObjectName(*descriptor_set, "name");
				#endif

				auto &write_descriptor_sets{m_writeDescriptorMap[frame_index].at(set)};

				// The image infos need to be here so they don't go out of scope when iterating through the resources
				std::vector<std::vector<vk::DescriptorImageInfo> > descriptor_image_infos;
				uint32                                             descriptor_image_info_index{0u};

				for (const auto &[binding, resource]: resources)
				{
					auto &stored_write_descriptor{write_descriptor_sets.at(binding)};

					vk::WriteDescriptorSet &write_descriptor{stored_write_descriptor.wds};
					write_descriptor.dstSet = descriptor_set;

					GPUResourceHandle resource_handle{resource.resources[0]};
					TST_ASSERT_MSG(resource_handle, "Why is ts nullptr");
					if (resource.type == EGPUResourceType::eTexture2D)
						_populateWriteDescriptorTexture2DArray(stored_write_descriptor, resource, descriptor_image_infos, descriptor_image_info_index, frame_index);
					else
					{
						stored_write_descriptor.resourceHandles[0] = resource_handle->getDescriptorResourceHandle(frame_index);
						resource_handle->populateWriteDescriptor(write_descriptor, frame_index);
					}
				}

				auto write_descriptors{
					write_descriptor_sets | std::views::values | std::ranges::to<std::vector>() | std::views::transform(&WriteDescriptor::wds) | std::ranges::to<
						std::vector>()
				};

				if (!write_descriptors.empty())
					m_device->getVulkanLogicalDevice().updateDescriptorSets(write_descriptors, {});
			}
		}
	}

	auto VKDescriptorSetManager::updateDescriptors(uint32 p_frame_index) -> void
	{
		for (const auto &[set, resources]: m_descriptorResources)
		{
			for (const auto &[binding, resource]: resources)
			{
				if (resource.resources[0].as<VKTexture2D>())
				{
					for (uint32 i{0u}; i < resource.resources.size(); ++i)
					{
						void *descriptor_resource_handle{resource.resources[i]->getDescriptorResourceHandle(p_frame_index)};
						if (descriptor_resource_handle != m_writeDescriptorMap[p_frame_index].at(set).at(binding).resourceHandles[i])
						{
							m_invalidDescriptorResources[set][binding] = resource;
							break;
						}
					}
				}
				else
				{
					void *descriptor_resource_handle{resource.resources[0]->getDescriptorResourceHandle(p_frame_index)};
					if (descriptor_resource_handle != m_writeDescriptorMap[p_frame_index].at(set).at(binding).resourceHandles[0])
						m_invalidDescriptorResources[set][binding] = resource;
				}
			}
		}

		if (m_invalidDescriptorResources.empty())
			return;

		for (const auto &[set, resources]: m_invalidDescriptorResources)
		{
			std::vector<vk::WriteDescriptorSet> write_descriptor_sets;
			write_descriptor_sets.reserve(resources.size());

			std::vector<std::vector<vk::DescriptorImageInfo> > descriptor_image_infos;
			uint32                                             descriptor_image_info_index{0u};
			for (const auto &[binding, resource]: resources)
			{
				auto &write_descriptor{m_writeDescriptorMap[p_frame_index].at(set).at(binding)};

				GPUResourceHandle resource_handle{resource.resources[0]};
				if (resource_handle.as<VKTexture2D>())
					_populateWriteDescriptorTexture2DArray(write_descriptor, resource, descriptor_image_infos, descriptor_image_info_index, p_frame_index);
				else
				{
					resource_handle->populateWriteDescriptor(write_descriptor.wds, p_frame_index);
					write_descriptor.resourceHandles[0] = resource_handle->getDescriptorResourceHandle(p_frame_index);
				}

				write_descriptor_sets.emplace_back(write_descriptor.wds);
			}

			m_device->getVulkanLogicalDevice().updateDescriptorSets(write_descriptor_sets, {});
		}
		m_invalidDescriptorResources.clear();
	}

	auto VKDescriptorSetManager::getDescriptorSets(uint32 p_frame_index) const -> std::vector<vk::DescriptorSet>
	{
		TST_ASSERT_MSG(p_frame_index < m_device->getSpecInfo().maxFramesInFlight, "Frame index out of bounds");
		std::vector<vk::DescriptorSet> result;
		for (auto &descriptor_set: m_descriptorSets[p_frame_index])
			result.emplace_back(*descriptor_set);
		return result;
	}

	auto VKDescriptorSetManager::getDescriptorDeclaration(const String &p_name) const -> const DescriptorDeclaration *
	{
		if (!m_descriptorDeclarations.contains(p_name))
			return nullptr;
		return &m_descriptorDeclarations.at(p_name);
	}

	auto VKDescriptorSetManager::getDescriptorDeclarations() const -> const std::unordered_map<String, DescriptorDeclaration> &
	{
		return m_descriptorDeclarations;
	}

	auto VKDescriptorSetManager::getWhiteTexture() const -> const Texture2DHandle &
	{
		return m_whiteTexture;
	}

	auto VKDescriptorSetManager::getWhiteTexture3D() const -> const Texture3DHandle &
	{
		return m_whiteTexture3D;
	}

	auto VKDescriptorSetManager::hasDescriptorSets() const -> bool
	{
		return !m_descriptorSets.empty() && !m_descriptorSets[0].empty();
	}

	auto VKDescriptorSetManager::getSpecInfo() const -> const DescriptorSetManagerSpecInfo &
	{
		return m_specInfo;
	}

	auto VKDescriptorSetManager::_populateWriteDescriptorTexture2DArray(WriteDescriptor &p_write_descriptor, const DescriptorResource &p_resource,
																		std::vector<std::vector<vk::DescriptorImageInfo> > &p_descriptor_image_infos,
																		uint32 &p_descriptor_image_info_index, uint32 p_frame_index) -> void
	{
		auto &current_image_infos{p_descriptor_image_infos.emplace_back()};
		current_image_infos.resize(p_resource.resources.size());

		for (uint32 i{0u}; i < p_resource.resources.size(); ++i)
		{
			current_image_infos[i]                = p_resource.resources[i].as<VKTexture2D>()->getDescriptorInfo();
			p_write_descriptor.resourceHandles[i] = p_resource.resources[i]->getDescriptorResourceHandle(p_frame_index);
		}

		p_write_descriptor.wds.pImageInfo = current_image_infos.data();
		++p_descriptor_image_info_index;
	}
}
