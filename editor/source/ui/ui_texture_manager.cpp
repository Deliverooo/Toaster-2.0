#include "ui_texture_manager.hpp"

namespace toaster::ui
{
	UITextureManager::UITextureManager(render::RenderContext *p_render_ctx, vk::DescriptorPool p_descriptor_pool) : m_renderCtx(p_render_ctx),
																													m_descriptorPool(p_descriptor_pool)
	{
		// Create descriptor set layout for textures
		vk::DescriptorSetLayoutBinding binding{};
		binding.descriptorType  = vk::DescriptorType::eCombinedImageSampler;
		binding.descriptorCount = 1;
		binding.stageFlags      = vk::ShaderStageFlagBits::eFragment;
		binding.binding         = 0;

		vk::DescriptorSetLayoutCreateInfo layout_info{};
		layout_info.bindingCount = 1;
		layout_info.pBindings    = &binding;

		m_descriptorSetLayout = m_renderCtx->getLogicalDevice()->getVulkanLogicalDevice().createDescriptorSetLayout(layout_info);
	}

	auto UITextureManager::registerOrGetTexture(const String &p_name, const gpu::Texture2DHandle &p_texture) -> ImTextureID
	{
		auto it{m_textureInfoMap.find(p_name)};
		if (it != m_textureInfoMap.end())
		{
			if (it->second.textureRef->getDescriptorInfo() != p_texture->getDescriptorInfo())
			{
				m_renderCtx->getLogicalDevice()->deferDestruction([device = m_renderCtx->getLogicalDevice(), ds = it->second.descriptorSet, pool = m_descriptorPool
																  ]() mutable -> void
																  {
																	  static_cast<vk::Device>(device->getVulkanLogicalDevice()).freeDescriptorSets(pool, {ds});
																  });
				it->second.descriptorSet = nullptr;
				it->second.textureRef.reset();
			}
			else
				return reinterpret_cast<ImTextureID>(static_cast<VkDescriptorSet>(m_textureInfoMap[p_name].descriptorSet));
		}

		// Allocate descriptor set
		vk::DescriptorSetAllocateInfo alloc_info{};
		alloc_info.descriptorPool     = m_descriptorPool;
		alloc_info.descriptorSetCount = 1;
		vk::DescriptorSetLayout dsl[]{*m_descriptorSetLayout};
		alloc_info.pSetLayouts = dsl;

		vk::DescriptorSet descriptor_set = ((vk::Device) m_renderCtx->getLogicalDevice()->getVulkanLogicalDevice()).allocateDescriptorSets(alloc_info).front();

		vk::DebugUtilsObjectNameInfoEXT name_info{};
		name_info.objectType   = vk::ObjectType::eDescriptorSet;
		name_info.objectHandle = (uint64) (VkDescriptorSet) descriptor_set;
		name_info.pObjectName  = p_name.c_str();
		m_renderCtx->getLogicalDevice()->getVulkanLogicalDevice().setDebugUtilsObjectNameEXT(name_info);

		vk::WriteDescriptorSet write_set{};
		write_set.dstSet          = descriptor_set;
		write_set.descriptorCount = 1;
		write_set.descriptorType  = vk::DescriptorType::eCombinedImageSampler;
		write_set.pImageInfo      = &p_texture->getDescriptorInfo();
		write_set.dstBinding      = 0;

		m_renderCtx->getLogicalDevice()->getVulkanLogicalDevice().updateDescriptorSets(write_set, {});

		// Store texture info
		m_textureInfoMap[p_name] = {
			p_texture,
			descriptor_set,
			ImVec2{static_cast<float32>(p_texture->getSpecInfo().width), static_cast<float32>(p_texture->getSpecInfo().height)}
		};

		// Return the descriptor set as ImTextureID
		return reinterpret_cast<ImTextureID>(static_cast<VkDescriptorSet>(m_textureInfoMap[p_name].descriptorSet));
	}

	auto UITextureManager::hasTexture(const String &p_name) const -> bool
	{
		return m_textureInfoMap.contains(p_name);
	}

	auto UITextureManager::getTexture(const String &p_name) const -> ImTextureID
	{
		if (!m_textureInfoMap.contains(p_name))
		{
		}

		return reinterpret_cast<ImTextureID>(static_cast<VkDescriptorSet>(m_textureInfoMap.at(p_name).descriptorSet));
	}

	auto UITextureManager::getTextureSize(const String &p_name) const -> ImVec2
	{
		if (!m_textureInfoMap.contains(p_name))
		{
			TST_PERMA_ASSERT(false);
		}

		return m_textureInfoMap.at(p_name).size;
	}
}
