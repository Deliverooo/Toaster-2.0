#include "vk_descriptor_set_manager.hpp"
#include "vk_gpu_context.hpp"

namespace toaster::gpu
{
	VKDescriptorSetManager::VKDescriptorSetManager(VKGPUContext *p_ctx, const RefPtr<VKShader> &p_shader, uint32 p_start_set, uint32 p_end_set) : m_ctx(p_ctx),
																																				  m_shader(p_shader),
																																				  m_startSet(p_start_set),
																																				  m_endSet(p_end_set)
	{
		TextureSpecInfo texture_spec_info{};
		texture_spec_info.width        = 1u;
		texture_spec_info.height       = 1u;
		texture_spec_info.format       = vk::Format::eR8G8B8A8Unorm;
		texture_spec_info.generateMips = false;

		uint32 texture_data{0xFFFFFFFF};
		m_whiteTexture = m_ctx->alloc<VKTexture2D>(texture_spec_info, &texture_data, sizeof(uint32));

		const auto &descriptor_sets{m_shader->getReflectedShaderDescriptorSets()};
		m_writeDescriptorMap.resize(VKGPUContext::c_maxFramesInFlight);

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

				if (descriptor_declaration.type == EDescriptorType::eSampler2D)
					for (uint32 i{0u}; i < descriptor_resource.resources.size(); ++i)
						descriptor_resource.resources[i] = m_whiteTexture.as<IGPUResource>();

				for (uint32 frame_index{0u}; frame_index < VKGPUContext::c_maxFramesInFlight; ++frame_index)
					m_writeDescriptorMap[frame_index][set][binding] = {write_descriptor, std::vector<void *>{write_descriptor.descriptorCount}};

				// if (descriptor_set.imageSamplers.contains(binding))
			}
		}
	}

	auto VKDescriptorSetManager::getContext() const -> VKGPUContext *
	{
		return m_ctx;
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

	auto VKDescriptorSetManager::setDescriptor(const String &p_name, const RefPtr<VKTexture2D> &p_texture_2d) -> void
	{
		if (const auto decl{getDescriptorDeclaration(p_name)})
			m_descriptorResources.at(decl->set)[decl->binding] = p_texture_2d;
		else
			LOG_WARN("Descriptor was not found: {}", p_name);
	}

	auto VKDescriptorSetManager::setDescriptor(const String &p_name, const RefPtr<VKTexture2D> &p_texture_2d, uint32 p_array_index) -> void
	{
		// TODO: texture arrays
		if (const auto decl{getDescriptorDeclaration(p_name)})
			m_descriptorResources.at(decl->set)[decl->binding] = p_texture_2d;
		else
			LOG_WARN("Descriptor was not found: {}", p_name);
	}

	auto VKDescriptorSetManager::bakeDescriptors() -> void
	{
		std::array<vk::DescriptorPoolSize, 2> descriptor_pool_sizes{
			vk::DescriptorPoolSize{vk::DescriptorType::eUniformBuffer, 100},
			vk::DescriptorPoolSize{vk::DescriptorType::eCombinedImageSampler, 100}
		};

		vk::DescriptorPoolCreateInfo descriptor_pool_create_info{};
		descriptor_pool_create_info.poolSizeCount = descriptor_pool_sizes.size();
		descriptor_pool_create_info.pPoolSizes    = descriptor_pool_sizes.data();
		descriptor_pool_create_info.maxSets       = 10u * VKGPUContext::c_maxFramesInFlight;
		descriptor_pool_create_info.flags         = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;

		m_descriptorPool = {m_ctx->getDevice(), descriptor_pool_create_info};

		if (m_descriptorSets.empty())
			for (uint32 i{0u}; i < VKGPUContext::c_maxFramesInFlight; ++i)
				m_descriptorSets.emplace_back();

		for (auto &descriptor_set: m_descriptorSets)
			descriptor_set.clear();

		for (const auto &[set, resources]: m_descriptorResources)
		{
			for (uint32 frame_index{0u}; frame_index < VKGPUContext::c_maxFramesInFlight; ++frame_index)
			{
				const vk::raii::DescriptorSetLayout &descriptor_set_layout{m_shader->getDescriptorSetLayout(set)};
				vk::DescriptorSetAllocateInfo        descriptor_set_allocate_info{};
				descriptor_set_allocate_info.descriptorPool     = m_descriptorPool;
				descriptor_set_allocate_info.descriptorSetCount = 1;
				descriptor_set_allocate_info.pSetLayouts        = &*descriptor_set_layout;

				auto &descriptor_set{
					m_descriptorSets[frame_index].emplace_back(std::move(m_ctx->getDevice().allocateDescriptorSets(descriptor_set_allocate_info).front()))
				};

				auto &write_descriptor_sets{m_writeDescriptorMap[frame_index].at(set)};
				for (const auto &[binding, resource]: resources)
				{
					auto &stored_write_descriptor{write_descriptor_sets.at(binding)};

					vk::WriteDescriptorSet &write_descriptor{stored_write_descriptor.wds};
					write_descriptor.dstSet = descriptor_set;

					switch (resource.type)
					{
						case EGPUResourceType::eUniformBuffer:
						{
							auto uniform_buffer{resource.resources[0].as<VKUniformBuffer>()};
							write_descriptor.pBufferInfo               = &uniform_buffer->getDescriptorInfo();
							stored_write_descriptor.resourceHandles[0] = write_descriptor.pBufferInfo->buffer;

							if (!write_descriptor.pBufferInfo->buffer)
								TST_ASSERT_MSG(false, "Oh no");
							break;
						}
						case EGPUResourceType::eUniformBufferPFF:
						{
							auto uniform_buffer{resource.resources[0].as<VKUniformBufferPFF>()};
							TST_ASSERT(uniform_buffer);
							write_descriptor.pBufferInfo               = &uniform_buffer->getUBO(frame_index).as<VKUniformBuffer>()->getDescriptorInfo();
							stored_write_descriptor.resourceHandles[0] = write_descriptor.pBufferInfo->buffer;

							if (!write_descriptor.pBufferInfo->buffer)
								TST_ASSERT_MSG(false, "Oh no");
							break;
						}
						case EGPUResourceType::eTexture2D:
						{
							auto texture_2d{resource.resources[0].as<VKTexture2D>()};
							TST_ASSERT(texture_2d);
							write_descriptor.pImageInfo                = &texture_2d->getDescriptorInfo();
							stored_write_descriptor.resourceHandles[0] = write_descriptor.pImageInfo->imageView;

							if (!write_descriptor.pImageInfo->imageView)
								TST_ASSERT_MSG(false, "Oh no");
							break;
						}

						default: break;
					}
				}

				std::vector<vk::WriteDescriptorSet> write_descriptors;
				for (auto &[binding, write_descriptor]: write_descriptor_sets)
				{
					write_descriptors.emplace_back(write_descriptor.wds);
				}

				if (!write_descriptors.empty())
				{
					LOG_INFO("Num descriptors: {} | Set: {}", write_descriptors.size(), set);
					m_ctx->getDevice().updateDescriptorSets(write_descriptors, {});
				}
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
						const auto &buffer_info{resource.resources[0].as<VKUniformBuffer>()->getDescriptorInfo()};
						if (buffer_info.buffer != m_writeDescriptorMap[p_frame_index].at(set).at(binding).resourceHandles[0])
							m_invalidDescriptorResources[set][binding] = resource;
						break;
					}
					case EGPUResourceType::eUniformBufferPFF:
					{
						const auto &buffer_info{resource.resources[0].as<VKUniformBufferPFF>()->getUBO(p_frame_index)->getDescriptorInfo()};
						if (buffer_info.buffer != m_writeDescriptorMap[p_frame_index].at(set).at(binding).resourceHandles[0])
							m_invalidDescriptorResources[set][binding] = resource;
						break;
					}
					case EGPUResourceType::eTexture2D:
					{
						for (uint32 i{0u}; i < resource.resources.size(); ++i)
						{
							auto texture_2d{resource.resources[i].as<VKTexture2D>()};
							if (!texture_2d)
								texture_2d = m_whiteTexture;
							const auto &image_info{texture_2d->getDescriptorInfo()};
							if (image_info.imageView != m_writeDescriptorMap[p_frame_index].at(set).at(binding).resourceHandles[0])
								m_invalidDescriptorResources[set][binding] = resource;
							break;
						}
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

			for (const auto &[binding, resource]: resources)
			{
				auto &write_descriptor{m_writeDescriptorMap[p_frame_index].at(set).at(binding)};

				switch (resource.type)
				{
					case EGPUResourceType::eUniformBuffer:
					{
						auto uniform_buffer{resource.resources[0].as<VKUniformBuffer>()};
						write_descriptor.wds.pBufferInfo    = &uniform_buffer->getDescriptorInfo();
						write_descriptor.resourceHandles[0] = uniform_buffer->getDescriptorInfo().buffer;
						break;
					}
					case EGPUResourceType::eUniformBufferPFF:
					{
						auto uniform_buffer{resource.resources[0].as<VKUniformBufferPFF>()};
						write_descriptor.wds.pBufferInfo    = &uniform_buffer->getUBO(p_frame_index)->getDescriptorInfo();
						write_descriptor.resourceHandles[0] = uniform_buffer->getUBO(p_frame_index)->getDescriptorInfo().buffer;
						break;
					}
					case EGPUResourceType::eTexture2D:
					{
						if (resource.resources.size() > 1)
						{
						}
						else
						{
							auto texture_2d{resource.resources[0].as<VKTexture2D>()};
							write_descriptor.wds.pImageInfo     = &texture_2d->getDescriptorInfo();
							write_descriptor.resourceHandles[0] = texture_2d->getDescriptorInfo().imageView;
						}
					}
					default: break;
				}

				write_descriptor_sets.emplace_back(write_descriptor.wds);
			}
			LOG_INFO("Descriptor count: {} | Set: {}", write_descriptor_sets.size(), set);
			m_ctx->getDevice().updateDescriptorSets(write_descriptor_sets, {});
		}
		m_invalidDescriptorResources.clear();
	}

	auto VKDescriptorSetManager::getDescriptorSets(uint32 p_frame_index) const -> std::vector<vk::DescriptorSet>
	{
		TST_ASSERT_MSG(p_frame_index < VKGPUContext::c_maxFramesInFlight, "Frame index out of bounds");
		TST_ASSERT(!m_descriptorSets.empty());
		std::vector<vk::DescriptorSet> result;
		for (auto &descriptor_set: m_descriptorSets[p_frame_index])
			result.emplace_back(*descriptor_set);
		TST_ASSERT(!result.empty());
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

	auto VKDescriptorSetManager::getWhiteTexture() const -> const RefPtr<VKTexture2D> &
	{
		return m_whiteTexture;
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

	auto VKDescriptorSetManager::_getDescriptorType(vk::DescriptorType p_type) const -> EDescriptorType
	{
		switch (p_type)
		{
			case vk::DescriptorType::eUniformBuffer: return EDescriptorType::eUniformBuffer;
			case vk::DescriptorType::eCombinedImageSampler:
			case vk::DescriptorType::eSampledImage:
				return EDescriptorType::eSampler2D;
			default: return EDescriptorType::eUnknown;
		}
		return EDescriptorType::eUnknown;
	}

	auto VKDescriptorSetManager::_getResourceType(vk::DescriptorType p_type) const -> EGPUResourceType
	{
		switch (p_type)
		{
			case vk::DescriptorType::eUniformBuffer: return EGPUResourceType::eUniformBuffer;
			case vk::DescriptorType::eCombinedImageSampler:
			case vk::DescriptorType::eSampledImage:
				return EGPUResourceType::eTexture2D;
			default: return EGPUResourceType::eUnknown;
		}
		return EGPUResourceType::eUnknown;
	}
}
