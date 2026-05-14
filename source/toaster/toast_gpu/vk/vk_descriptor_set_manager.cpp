#include "vk_descriptor_set_manager.hpp"

#include <ranges>

#include "vk_logical_device.hpp"

namespace toaster::gpu
{
	VKDescriptorSetManager::VKDescriptorSetManager(VKLogicalDevice *p_device, const RefPtr<VKShader> &p_shader, uint32 p_start_set,
												   uint32           p_end_set) : m_device(p_device), m_shader(p_shader), m_startSet(p_start_set), m_endSet(p_end_set)
	{
		TextureSpecInfo texture_spec_info{};
		texture_spec_info.width        = 1u;
		texture_spec_info.height       = 1u;
		texture_spec_info.format       = vk::Format::eR8G8B8A8Unorm;
		texture_spec_info.generateMips = false;
		uint32 texture_data{0xFFFFFFFF};
		m_whiteTexture = make_unique<VKTexture2D>(m_device, texture_spec_info, &texture_data, sizeof(uint32));

		uint32 texture_3d_data[6]{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
		m_whiteTexture3D = make_unique<VKTexture3D>(m_device, texture_spec_info);
		m_whiteTexture3D->setData(Buffer{texture_3d_data, sizeof(uint32) * 6});

		const auto &descriptor_sets{m_shader->getReflectedShaderDescriptorSets()};
		m_writeDescriptorMap.resize(m_device->getSpecInfo().maxFramesInFlight);

		for (uint32 set{m_startSet}; set <= m_endSet; ++set)
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
				descriptor_declaration.type      = _getDescriptorType(write_descriptor.descriptorType);

				DescriptorResource &descriptor_resource{m_descriptorResources[set][binding]};
				descriptor_resource.resources.resize(write_descriptor.descriptorCount);
				descriptor_resource.type = _getResourceType(write_descriptor.descriptorType);

				if (descriptor_set.imageSamplers.contains(binding))
				{
					auto & sampler{descriptor_set.imageSamplers.at(binding)};
					uint32 dimension{sampler.dimension};
					if (write_descriptor.descriptorType == vk::DescriptorType::eSampledImage || write_descriptor.descriptorType ==
						vk::DescriptorType::eCombinedImageSampler)
					{
						switch (dimension)
						{
							case 1:
								break;
							case 2: descriptor_declaration.type = EDescriptorType::eSampler2D;
								break;
							case 3: descriptor_declaration.type = EDescriptorType::eSampler3D;
								break;
							default: break;
						}
					}
					else if (write_descriptor.descriptorType == vk::DescriptorType::eStorageImage)
					{
						switch (dimension)
						{
							case 1:
								break;
							case 2: descriptor_declaration.type = EDescriptorType::eImage2D;
								break;
							case 3: descriptor_declaration.type = EDescriptorType::eImage3D;
								break;
							default: break;
						}
					}
				}

				if (descriptor_declaration.type == EDescriptorType::eSampler2D)
					for (uint32 i{0u}; i < descriptor_resource.resources.size(); ++i)
						descriptor_resource.resources[i] = dynamic_cast<const IGPUResource *>(m_whiteTexture.get());
				else if (descriptor_declaration.type == EDescriptorType::eSampler3D)
					for (uint32 i{0u}; i < descriptor_resource.resources.size(); ++i)
						descriptor_resource.resources[i] = dynamic_cast<const IGPUResource *>(m_whiteTexture3D.get());

				for (uint32 frame_index{0u}; frame_index < m_device->getSpecInfo().maxFramesInFlight; ++frame_index)
					m_writeDescriptorMap[frame_index][set][binding] = {write_descriptor, std::vector<void *>{write_descriptor.descriptorCount}};
			}
		}
	}


	auto VKDescriptorSetManager::setDescriptor(const String &p_name, const VKUniformBuffer *p_uniform_buffer) -> void
	{
		if (const auto decl{getDescriptorDeclaration(p_name)})
			m_descriptorResources.at(decl->set)[decl->binding] = p_uniform_buffer;
		else
			LOG_WARN("Descriptor was not found: {}", p_name);
	}

	auto VKDescriptorSetManager::setDescriptor(const String &p_name, const VKUniformBufferPFF *p_uniform_buffer_pff) -> void
	{
		if (const auto decl{getDescriptorDeclaration(p_name)})
			m_descriptorResources.at(decl->set)[decl->binding] = p_uniform_buffer_pff;
		else
			LOG_WARN("Descriptor was not found: {}", p_name);
	}

	auto VKDescriptorSetManager::setDescriptor(const String &p_name, const VKStorageBuffer *p_storage_buffer) -> void
	{
		if (const auto decl{getDescriptorDeclaration(p_name)})
			m_descriptorResources.at(decl->set)[decl->binding] = p_storage_buffer;
		else
			LOG_WARN("Descriptor was not found: {}", p_name);
	}

	auto VKDescriptorSetManager::setDescriptor(const String &p_name, const VKStorageBufferPFF *p_storage_buffer_pff) -> void
	{
		if (const auto decl{getDescriptorDeclaration(p_name)})
			m_descriptorResources.at(decl->set)[decl->binding] = p_storage_buffer_pff;
		else
			LOG_WARN("Descriptor was not found: {}", p_name);
	}

	auto VKDescriptorSetManager::setDescriptor(const String &p_name, const VKTexture2D *p_texture_2d) -> void
	{
		if (const auto decl{getDescriptorDeclaration(p_name)})
			m_descriptorResources.at(decl->set)[decl->binding] = p_texture_2d;
		else
			LOG_WARN("Descriptor was not found: {}", p_name);
	}

	auto VKDescriptorSetManager::setDescriptor(const String &p_name, const VKTexture2D *p_texture_2d, uint32 p_array_index) -> void
	{
		const auto decl{getDescriptorDeclaration(p_name)};
		TST_ASSERT_MSG(p_array_index < decl->arraySize, "Out of bounds");
		if (decl)
			m_descriptorResources.at(decl->set)[decl->binding].set(p_texture_2d, p_array_index);
		else
			LOG_WARN("Descriptor was not found: {}", p_name);
	}

	auto VKDescriptorSetManager::setDescriptor(const String &p_name, const VKImage2D *p_image_2d) -> void
	{
		if (const auto decl{getDescriptorDeclaration(p_name)})
			m_descriptorResources.at(decl->set)[decl->binding] = p_image_2d;
		else
			LOG_WARN("Descriptor was not found: {}", p_name);
	}

	auto VKDescriptorSetManager::setDescriptor(const String &p_name, const VKTexture3D *p_texture_3d) -> void
	{
		if (const auto decl{getDescriptorDeclaration(p_name)})
			m_descriptorResources.at(decl->set)[decl->binding] = p_texture_3d;
		else
			LOG_WARN("Descriptor was not found: {}", p_name);
	}

	auto VKDescriptorSetManager::setDescriptor(const String &p_name, const RefPtr<VKUniformBuffer> &p_uniform_buffer) -> void
	{
		if (const auto decl{getDescriptorDeclaration(p_name)})
			m_descriptorResources.at(decl->set)[decl->binding] = p_uniform_buffer;
		else
			LOG_WARN("Descriptor was not found: {}", p_name);
	}

	auto VKDescriptorSetManager::setDescriptor(const String &p_name, const RefPtr<VKUniformBufferPFF> &p_uniform_buffer_pff) -> void
	{
		if (const auto decl{getDescriptorDeclaration(p_name)})
			m_descriptorResources.at(decl->set)[decl->binding] = p_uniform_buffer_pff;
		else
			LOG_WARN("Descriptor was not found: {}", p_name);
	}

	auto VKDescriptorSetManager::setDescriptor(const String &p_name, const RefPtr<VKStorageBuffer> &p_storage_buffer) -> void
	{
		if (const auto decl{getDescriptorDeclaration(p_name)})
			m_descriptorResources.at(decl->set)[decl->binding] = p_storage_buffer;
		else
			LOG_WARN("Descriptor was not found: {}", p_name);
	}

	auto VKDescriptorSetManager::setDescriptor(const String &p_name, const RefPtr<VKStorageBufferPFF> &p_storage_buffer_pff) -> void
	{
		if (const auto decl{getDescriptorDeclaration(p_name)})
			m_descriptorResources.at(decl->set)[decl->binding] = p_storage_buffer_pff;
		else
			LOG_WARN("Descriptor was not found: {}", p_name);
	}

	auto VKDescriptorSetManager::setDescriptor(const String &p_name, const RefPtr<VKTexture2D> &p_texture_2d) -> void
	{
		if (const auto decl{getDescriptorDeclaration(p_name)})
			m_descriptorResources.at(decl->set)[decl->binding] = p_texture_2d;
		else
			LOG_WARN("Descriptor was not found: {}", p_name);
	}

	auto VKDescriptorSetManager::setDescriptor(const String &p_name, const RefPtr<VKTexture2D> &p_texture_2d, uint32 p_array_index) -> void
	{
		const auto decl{getDescriptorDeclaration(p_name)};
		TST_ASSERT_MSG(p_array_index < decl->arraySize, "Out of bounds");
		if (decl)
			m_descriptorResources.at(decl->set)[decl->binding].set(p_texture_2d, p_array_index);
		else
			LOG_WARN("Descriptor was not found: {}", p_name);
	}

	auto VKDescriptorSetManager::setDescriptor(const String &p_name, const RefPtr<VKImage2D> &p_image_2d) -> void
	{
		if (const auto decl{getDescriptorDeclaration(p_name)})
			m_descriptorResources.at(decl->set)[decl->binding] = p_image_2d;
		else
			LOG_WARN("Descriptor was not found: {}", p_name);
	}

	auto VKDescriptorSetManager::setDescriptor(const String &p_name, const RefPtr<VKTexture3D> &p_texture_3d) -> void
	{
		if (const auto decl{getDescriptorDeclaration(p_name)})
			m_descriptorResources.at(decl->set)[decl->binding] = p_texture_3d;
		else
			LOG_WARN("Descriptor was not found: {}", p_name);
	}

	auto VKDescriptorSetManager::bakeDescriptors() -> void
	{
		std::array<vk::DescriptorPoolSize, 5> descriptor_pool_sizes{
			vk::DescriptorPoolSize{vk::DescriptorType::eUniformBuffer, 100},
			vk::DescriptorPoolSize{vk::DescriptorType::eStorageBuffer, 100},
			vk::DescriptorPoolSize{vk::DescriptorType::eCombinedImageSampler, 100},
			vk::DescriptorPoolSize{vk::DescriptorType::eSampledImage, 100},
			vk::DescriptorPoolSize{vk::DescriptorType::eStorageImage, 100}
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
				const vk::raii::DescriptorSetLayout &descriptor_set_layout{m_shader->getDescriptorSetLayout(set)};
				vk::DescriptorSetAllocateInfo        descriptor_set_allocate_info{};
				descriptor_set_allocate_info.descriptorPool     = m_descriptorPool;
				descriptor_set_allocate_info.descriptorSetCount = 1;
				descriptor_set_allocate_info.pSetLayouts        = &*descriptor_set_layout;

				auto &descriptor_set{
					m_descriptorSets[frame_index].emplace_back(std::move(m_device->getVulkanLogicalDevice().allocateDescriptorSets(descriptor_set_allocate_info).front()))
				};

				auto &                                             write_descriptor_sets{m_writeDescriptorMap[frame_index].at(set)};
				std::vector<std::vector<vk::DescriptorImageInfo> > descriptor_image_infos;
				uint32                                             descriptor_image_info_index{0u};
				for (const auto &[binding, resource]: resources)
				{
					auto &stored_write_descriptor{write_descriptor_sets.at(binding)};

					vk::WriteDescriptorSet &write_descriptor{stored_write_descriptor.wds};
					write_descriptor.dstSet = descriptor_set;

					switch (resource.type)
					{
						case EGPUResourceType::eUniformBuffer:
						{
							auto uniform_buffer{dynamic_cast<const VKUniformBuffer *>(resource.resources[0])};
							write_descriptor.pBufferInfo               = &uniform_buffer->getDescriptorInfo();
							stored_write_descriptor.resourceHandles[0] = write_descriptor.pBufferInfo->buffer;

							if (!write_descriptor.pBufferInfo->buffer)
								TST_ASSERT_MSG(false, "Oh no");
							break;
						}
						case EGPUResourceType::eUniformBufferPFF:
						{
							auto uniform_buffer{dynamic_cast<const VKUniformBufferPFF *>(resource.resources[0])};
							TST_ASSERT(uniform_buffer);
							write_descriptor.pBufferInfo               = &uniform_buffer->getDescriptorInfo(frame_index);
							stored_write_descriptor.resourceHandles[0] = write_descriptor.pBufferInfo->buffer;

							if (!write_descriptor.pBufferInfo->buffer)
								TST_ASSERT_MSG(false, "Oh no");
							break;
						}
						case EGPUResourceType::eStorageBuffer:
						{
							auto storage_buffer{dynamic_cast<const VKStorageBuffer *>(resource.resources[0])};
							write_descriptor.pBufferInfo               = &storage_buffer->getDescriptorInfo();
							stored_write_descriptor.resourceHandles[0] = write_descriptor.pBufferInfo->buffer;

							if (!write_descriptor.pBufferInfo->buffer)
								TST_ASSERT_MSG(false, "Oh no");
							break;
						}
						case EGPUResourceType::eStorageBufferPFF:
						{
							auto storage_buffer{dynamic_cast<const VKStorageBufferPFF *>(resource.resources[0])};
							TST_ASSERT(storage_buffer);
							write_descriptor.pBufferInfo               = &storage_buffer->getSSBO(frame_index)->getDescriptorInfo();
							stored_write_descriptor.resourceHandles[0] = write_descriptor.pBufferInfo->buffer;

							if (!write_descriptor.pBufferInfo->buffer)
								TST_ASSERT_MSG(false, "Oh no");
							break;
						}
						case EGPUResourceType::eTexture2D:
						{
							if (resource.resources.size() > 1)
							{
								descriptor_image_infos.emplace_back(resource.resources.size());
								for (uint32 i{0u}; i < resource.resources.size(); ++i)
									descriptor_image_infos[descriptor_image_info_index][i] = dynamic_cast<const VKTexture2D *>(resource.resources[0])->
											getDescriptorInfo();
								write_descriptor.pImageInfo = descriptor_image_infos[descriptor_image_info_index].data();
								++descriptor_image_info_index;
							}
							else
							{
								auto texture_2d{dynamic_cast<const VKTexture2D *>(resource.resources[0])};
								write_descriptor.pImageInfo = &texture_2d->getDescriptorInfo();
							}

							stored_write_descriptor.resourceHandles[0] = write_descriptor.pImageInfo->imageView;

							if (!write_descriptor.pImageInfo->imageView)
								TST_ASSERT_MSG(false, "Oh no");
							break;
						}
						case EGPUResourceType::eImage2D:
						{
							auto image_2d{dynamic_cast<const VKImage2D *>(resource.resources[0])};
							TST_ASSERT(image_2d);
							write_descriptor.pImageInfo                = &image_2d->getDescriptorInfo();
							stored_write_descriptor.resourceHandles[0] = write_descriptor.pImageInfo->imageView;

							if (!write_descriptor.pImageInfo->imageView)
								TST_ASSERT_MSG(false, "Oh no");
							break;
						}
						case EGPUResourceType::eTexture3D:
						{
							auto texture_3d{dynamic_cast<const VKTexture3D *>(resource.resources[0])};
							TST_ASSERT(texture_3d);
							write_descriptor.pImageInfo                = &texture_3d->getDescriptorInfo();
							stored_write_descriptor.resourceHandles[0] = write_descriptor.pImageInfo->imageView;

							if (!write_descriptor.pImageInfo->imageView)
								TST_ASSERT_MSG(false, "Oh no");
							break;
						}

						default: break;
					}
				}

				std::vector<vk::WriteDescriptorSet> write_descriptors;
				for (auto &write_descriptor: write_descriptor_sets | std::views::values)
					write_descriptors.emplace_back(write_descriptor.wds);

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
				switch (resource.type)
				{
					case EGPUResourceType::eUniformBuffer:
					{
						const auto &buffer_info{dynamic_cast<const VKUniformBuffer *>(resource.resources[0])->getDescriptorInfo()};
						if (buffer_info.buffer != m_writeDescriptorMap[p_frame_index].at(set).at(binding).resourceHandles[0])
							m_invalidDescriptorResources[set][binding] = resource;
						break;
					}
					case EGPUResourceType::eUniformBufferPFF:
					{
						const auto &buffer_info{dynamic_cast<const VKUniformBufferPFF *>(resource.resources[0])->getDescriptorInfo(p_frame_index)};
						if (buffer_info.buffer != m_writeDescriptorMap[p_frame_index].at(set).at(binding).resourceHandles[0])
							m_invalidDescriptorResources[set][binding] = resource;
						break;
					}
					case EGPUResourceType::eStorageBuffer:
					{
						const auto &buffer_info{dynamic_cast<const VKStorageBuffer *>(resource.resources[0])->getDescriptorInfo()};
						if (buffer_info.buffer != m_writeDescriptorMap[p_frame_index].at(set).at(binding).resourceHandles[0])
							m_invalidDescriptorResources[set][binding] = resource;
						break;
					}
					case EGPUResourceType::eStorageBufferPFF:
					{
						const auto &buffer_info{dynamic_cast<const VKStorageBufferPFF *>(resource.resources[0])->getSSBO(p_frame_index)->getDescriptorInfo()};
						if (buffer_info.buffer != m_writeDescriptorMap[p_frame_index].at(set).at(binding).resourceHandles[0])
							m_invalidDescriptorResources[set][binding] = resource;
						break;
					}
					case EGPUResourceType::eTexture2D:
					{
						for (uint32 i{0u}; i < resource.resources.size(); ++i)
						{
							auto texture_2d{dynamic_cast<const VKTexture2D *>(resource.resources[i])};
							if (!texture_2d)
								texture_2d = m_whiteTexture.get();
							const auto &image_info{texture_2d->getDescriptorInfo()};
							if (image_info.imageView != m_writeDescriptorMap[p_frame_index].at(set).at(binding).resourceHandles[i])
							{
								m_invalidDescriptorResources[set][binding] = resource;
								break;
							}
						}
						break;
					}
					case EGPUResourceType::eImage2D:
					{
						const auto &image_info{dynamic_cast<const VKImage2D *>(resource.resources[0])->getDescriptorInfo()};
						if (image_info.imageView != m_writeDescriptorMap[p_frame_index].at(set).at(binding).resourceHandles[0])
							m_invalidDescriptorResources[set][binding] = resource;
						break;
					}
					case EGPUResourceType::eTexture3D:
					{
						const auto &image_info{dynamic_cast<const VKTexture3D *>(resource.resources[0])->getDescriptorInfo()};
						if (image_info.imageView != m_writeDescriptorMap[p_frame_index].at(set).at(binding).resourceHandles[0])
							m_invalidDescriptorResources[set][binding] = resource;
						break;
					}
					default: break;
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

				switch (resource.type)
				{
					case EGPUResourceType::eUniformBuffer:
					{
						auto uniform_buffer{dynamic_cast<const VKUniformBuffer *>(resource.resources[0])};
						write_descriptor.wds.pBufferInfo    = &uniform_buffer->getDescriptorInfo();
						write_descriptor.resourceHandles[0] = uniform_buffer->getDescriptorInfo().buffer;
						break;
					}
					case EGPUResourceType::eUniformBufferPFF:
					{
						auto uniform_buffer{dynamic_cast<const VKUniformBufferPFF *>(resource.resources[0])};
						write_descriptor.wds.pBufferInfo    = &uniform_buffer->getDescriptorInfo(p_frame_index);
						write_descriptor.resourceHandles[0] = uniform_buffer->getDescriptorInfo(p_frame_index).buffer;
						break;
					}
					case EGPUResourceType::eStorageBuffer:
					{
						auto storage_buffer{dynamic_cast<const VKStorageBuffer *>(resource.resources[0])};
						write_descriptor.wds.pBufferInfo    = &storage_buffer->getDescriptorInfo();
						write_descriptor.resourceHandles[0] = storage_buffer->getDescriptorInfo().buffer;
						break;
					}
					case EGPUResourceType::eStorageBufferPFF:
					{
						auto storage_buffer{dynamic_cast<const VKStorageBufferPFF *>(resource.resources[0])};
						write_descriptor.wds.pBufferInfo    = &storage_buffer->getSSBO(p_frame_index)->getDescriptorInfo();
						write_descriptor.resourceHandles[0] = storage_buffer->getSSBO(p_frame_index)->getDescriptorInfo().buffer;
						break;
					}
					case EGPUResourceType::eTexture2D:
					{
						if (resource.resources.size() > 1)
						{
							descriptor_image_infos.emplace_back(resource.resources.size());
							for (uint32 i{0u}; i < resource.resources.size(); ++i)
							{
								auto texture_2d{dynamic_cast<const VKTexture2D *>(resource.resources[i])};
								descriptor_image_infos[descriptor_image_info_index][i] = texture_2d->getDescriptorInfo();
								write_descriptor.resourceHandles[i]                    = descriptor_image_infos[descriptor_image_info_index][i].imageView;
							}
							write_descriptor.wds.pImageInfo = descriptor_image_infos[descriptor_image_info_index].data();
							++descriptor_image_info_index;
						}
						else
						{
							auto texture_2d{dynamic_cast<const VKTexture2D *>(resource.resources[0])};
							write_descriptor.wds.pImageInfo     = &texture_2d->getDescriptorInfo();
							write_descriptor.resourceHandles[0] = texture_2d->getDescriptorInfo().imageView;
						}
						break;
					}
					case EGPUResourceType::eImage2D:
					{
						auto image_2d{dynamic_cast<const VKImage2D *>(resource.resources[0])};
						write_descriptor.wds.pImageInfo     = &image_2d->getDescriptorInfo();
						write_descriptor.resourceHandles[0] = image_2d->getDescriptorInfo().imageView;
						break;
					}
					case EGPUResourceType::eTexture3D:
					{
						auto texture_3d{dynamic_cast<const VKTexture3D *>(resource.resources[0])};
						write_descriptor.wds.pImageInfo     = &texture_3d->getDescriptorInfo();
						write_descriptor.resourceHandles[0] = texture_3d->getDescriptorInfo().imageView;
						break;
					}
					default: break;
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

	auto VKDescriptorSetManager::getWhiteTexture() const -> const VKTexture2D *
	{
		return m_whiteTexture.get();
	}

	auto VKDescriptorSetManager::getWhiteTexture3D() const -> const VKTexture3D *
	{
		return m_whiteTexture3D.get();
	}

	auto VKDescriptorSetManager::hasDescriptorSets() const -> bool
	{
		return !m_descriptorSets.empty() && !m_descriptorSets[0].empty();
	}

	auto VKDescriptorSetManager::getStartSetIndex() const -> uint32
	{
		return m_startSet;
	}

	auto VKDescriptorSetManager::getEndSetIndex() const -> uint32
	{
		return m_endSet;
	}

	auto VKDescriptorSetManager::_getDescriptorType(vk::DescriptorType p_type) -> EDescriptorType
	{
		switch (p_type)
		{
			case vk::DescriptorType::eUniformBuffer: return EDescriptorType::eUniformBuffer;
			case vk::DescriptorType::eStorageBuffer: return EDescriptorType::eStorageBuffer;
			case vk::DescriptorType::eCombinedImageSampler:
			case vk::DescriptorType::eSampledImage:
				return EDescriptorType::eSampler2D;
			case vk::DescriptorType::eStorageImage:
				return EDescriptorType::eImage2D;
			default: return EDescriptorType::eUnknown;
		}
		return EDescriptorType::eUnknown;
	}

	auto VKDescriptorSetManager::_getResourceType(vk::DescriptorType p_type) -> EGPUResourceType
	{
		switch (p_type)
		{
			case vk::DescriptorType::eUniformBuffer: return EGPUResourceType::eUniformBuffer;
			case vk::DescriptorType::eStorageBuffer: return EGPUResourceType::eStorageBuffer;
			case vk::DescriptorType::eCombinedImageSampler:
			case vk::DescriptorType::eSampledImage:
				return EGPUResourceType::eTexture2D;
			case vk::DescriptorType::eStorageImage:
				return EGPUResourceType::eImage2D;
			default: return EGPUResourceType::eUnknown;
		}
		return EGPUResourceType::eUnknown;
	}
}
