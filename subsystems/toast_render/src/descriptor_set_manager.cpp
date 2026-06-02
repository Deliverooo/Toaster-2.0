#include "toast_render/descriptor_set_manager.hpp"

#include <ranges>

#include "toast_render/globals.hpp"
#include "toast_render/render_context.hpp"

namespace toaster::render
{
	DescriptorSetManager::DescriptorSetManager(RenderContext &p_render_ctx, const DescriptorSetManagerSpecInfo &p_spec_info) : m_renderCtx(&p_render_ctx),
																															   m_specInfo(p_spec_info)
	{
		const auto &descriptor_sets{m_specInfo.shader->getReflectedShaderDescriptorSets()};
		m_writeDescriptorMap.resize(RenderContext::maxFramesInFlight);

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
				descriptor_declaration.type = gpu::getDescriptorType(write_descriptor.descriptorType);

				gpu::DescriptorResource &descriptor_resource{m_descriptorResources[set][binding]};
				descriptor_resource.resources.resize(write_descriptor.descriptorCount);
				descriptor_resource.type = gpu::getResourceType(write_descriptor.descriptorType);

				// For samplers, they are different types depending on their dimension
				if (descriptor_set.imageSamplers.contains(binding))
				{
					auto &sampler{descriptor_set.imageSamplers.at(binding)};
					descriptor_declaration.type = gpu::getDescriptorImageSamplerType(write_descriptor.descriptorType, sampler.dimension);

					if (sampler.dimension == gpu::reflection::EImageDimension::e3D || sampler.dimension == gpu::reflection::EImageDimension::eCube)
						descriptor_resource.type = gpu::EGPUResourceType::eTexture3D;
				}
				else if (descriptor_set.storageImages.contains(binding))
				{
					auto &storage_image{descriptor_set.storageImages.at(binding)};
					descriptor_declaration.type = gpu::getDescriptorImageSamplerType(write_descriptor.descriptorType, storage_image.dimension);

					if (storage_image.dimension == gpu::reflection::EImageDimension::e3D || storage_image.dimension == gpu::reflection::EImageDimension::eCube)
						descriptor_resource.type = gpu::EGPUResourceType::eTexture3D;
				}

				// LOG_ERROR("Type: {}", descriptorTypeToString(descriptor_declaration.type));
				TST_ASSERT(descriptor_declaration.type != gpu::EDescriptorType::eUnknown);

				// Create default resources so you can actually set the resource descriptor during rendering
				if (descriptor_declaration.type == gpu::EDescriptorType::eSampler2D)
				{
					for (uint32 i{0u}; i < descriptor_resource.resources.size(); ++i)
						descriptor_resource.resources[i] = m_renderCtx->getGlobals()->whiteTexture().as<gpu::IGPUResource>();
				}
				else if (descriptor_declaration.type == gpu::EDescriptorType::eSampler3D || descriptor_declaration.type == gpu::EDescriptorType::eImage3D)
				{
					// I actually don't think I even support 3D samplers yet...
					for (uint32 i{0u}; i < descriptor_resource.resources.size(); ++i)
						descriptor_resource.resources[i] = m_renderCtx->getGlobals()->whiteTexture3D().as<gpu::IGPUResource>();
				}

				for (uint32 frame_index{0u}; frame_index < RenderContext::maxFramesInFlight; ++frame_index)
					m_writeDescriptorMap[frame_index][set][binding] = {write_descriptor, std::vector<void *>{write_descriptor.descriptorCount}};
			}
		}
	}

	auto DescriptorSetManager::bakeDescriptors() -> void
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
		descriptor_pool_create_info.maxSets       = 10u * RenderContext::maxFramesInFlight;
		descriptor_pool_create_info.flags         = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;

		m_descriptorPool = {*m_renderCtx->getLogicalDevice(), descriptor_pool_create_info};

		if (m_descriptorSets.empty())
			for (uint32 i{0u}; i < RenderContext::maxFramesInFlight; ++i)
				m_descriptorSets.emplace_back();

		for (auto &descriptor_set: m_descriptorSets)
			descriptor_set.clear();

		for (const auto &[set, resources]: m_descriptorResources)
		{
			for (uint32 frame_index{0u}; frame_index < RenderContext::maxFramesInFlight; ++frame_index)
			{
				const vk::raii::DescriptorSetLayout &descriptor_set_layout{m_specInfo.shader->getDescriptorSetLayout(set)};
				vk::DescriptorSetAllocateInfo        descriptor_set_allocate_info{};
				descriptor_set_allocate_info.descriptorPool     = m_descriptorPool;
				descriptor_set_allocate_info.descriptorSetCount = 1;
				descriptor_set_allocate_info.pSetLayouts        = &*descriptor_set_layout;

				auto &descriptor_set{
					m_descriptorSets[frame_index].emplace_back(std::move(m_renderCtx->getLogicalDevice()->getVulkanLogicalDevice().
																		 allocateDescriptorSets(descriptor_set_allocate_info).front()))
				};

				#ifndef NDEBUG
				m_renderCtx->getLogicalDevice()->setDebugObjectName(*descriptor_set, "name");
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

					gpu::GPUResourceHandle resource_handle{resource.resources[0]};
					TST_ASSERT_MSG(resource_handle, "Why is ts nullptr");
					if (resource.type == gpu::EGPUResourceType::eTexture2D)
						_populateWriteDescriptorTexture2DArray(stored_write_descriptor, resource, descriptor_image_infos, descriptor_image_info_index, frame_index);
					else
					{
						stored_write_descriptor.resourceHandles[0] = resource_handle->getDescriptorResourceHandle(frame_index);
						resource_handle->populateWriteDescriptor(write_descriptor, frame_index);
					}
				}

				auto write_descriptors{
					write_descriptor_sets | std::views::values | std::ranges::to<std::vector>() | std::views::transform(&gpu::WriteDescriptor::wds) | std::ranges::to<
						std::vector>()
				};

				if (!write_descriptors.empty())
					m_renderCtx->getLogicalDevice()->getVulkanLogicalDevice().updateDescriptorSets(write_descriptors, {});
			}
		}
	}

	auto DescriptorSetManager::updateDescriptors(uint32 p_frame_index) -> void
	{
		for (const auto &[set, resources]: m_descriptorResources)
		{
			for (const auto &[binding, resource]: resources)
			{
				if (resource.resources[0].as<gpu::VKTexture2D>())
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

				gpu::GPUResourceHandle resource_handle{resource.resources[0]};
				if (resource_handle.as<gpu::VKTexture2D>())
					_populateWriteDescriptorTexture2DArray(write_descriptor, resource, descriptor_image_infos, descriptor_image_info_index, p_frame_index);
				else
				{
					resource_handle->populateWriteDescriptor(write_descriptor.wds, p_frame_index);
					write_descriptor.resourceHandles[0] = resource_handle->getDescriptorResourceHandle(p_frame_index);
				}

				write_descriptor_sets.emplace_back(write_descriptor.wds);
			}

			m_renderCtx->getLogicalDevice()->getVulkanLogicalDevice().updateDescriptorSets(write_descriptor_sets, {});
		}
		m_invalidDescriptorResources.clear();
	}

	auto DescriptorSetManager::getDescriptorSets(uint32 p_frame_index) const -> std::vector<vk::DescriptorSet>
	{
		TST_ASSERT_MSG(p_frame_index <RenderContext::maxFramesInFlight, "Frame index out of bounds");
		std::vector<vk::DescriptorSet> result;
		for (auto &descriptor_set: m_descriptorSets[p_frame_index])
			result.emplace_back(*descriptor_set);
		return result;
	}

	auto DescriptorSetManager::getDescriptorDeclaration(const String &p_name) const -> const DescriptorDeclaration *
	{
		if (!m_descriptorDeclarations.contains(p_name))
			return nullptr;
		return &m_descriptorDeclarations.at(p_name);
	}

	auto DescriptorSetManager::getDescriptorDeclarations() const -> const std::unordered_map<String, DescriptorDeclaration> &
	{
		return m_descriptorDeclarations;
	}

	auto DescriptorSetManager::hasDescriptorSets() const -> bool
	{
		return !m_descriptorSets.empty() && !m_descriptorSets[0].empty();
	}

	auto DescriptorSetManager::getSpecInfo() const -> const DescriptorSetManagerSpecInfo &
	{
		return m_specInfo;
	}

	auto DescriptorSetManager::_populateWriteDescriptorTexture2DArray(gpu::WriteDescriptor &p_write_descriptor, const gpu::DescriptorResource &p_resource,
																	  std::vector<std::vector<vk::DescriptorImageInfo> > &p_descriptor_image_infos,
																	  uint32 &p_descriptor_image_info_index, uint32 p_frame_index) -> void
	{
		auto &current_image_infos{p_descriptor_image_infos.emplace_back()};
		current_image_infos.resize(p_resource.resources.size());

		for (uint32 i{0u}; i < p_resource.resources.size(); ++i)
		{
			current_image_infos[i]                = p_resource.resources[i].as<gpu::VKTexture2D>()->getDescriptorInfo();
			p_write_descriptor.resourceHandles[i] = p_resource.resources[i]->getDescriptorResourceHandle(p_frame_index);
		}

		p_write_descriptor.wds.pImageInfo = current_image_infos.data();
		++p_descriptor_image_info_index;
	}
}
