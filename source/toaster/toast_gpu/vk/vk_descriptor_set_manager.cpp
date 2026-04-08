#include "vk_descriptor_set_manager.hpp"
#include "vk_gpu_context.hpp"

namespace toaster::gpu
{
	VKDescriptorSetManager::VKDescriptorSetManager(VKGPUContext *p_ctx, const RefPtr<VKShader> &p_shader, uint32 p_start_set, uint32 p_end_set) : m_ctx(p_ctx),
																																				  m_shader(p_shader),
																																				  m_startSet(p_start_set),
																																				  m_endSet(p_end_set)
	{
		const auto &descriptor_sets{m_shader->getReflectedShaderDescriptorSets()};
		m_writeDescriptorMap.resize(VKGPUContext::c_maxFramesInFlight);

		for (uint32 set{m_startSet}; set <= m_endSet; ++set)
		{
			if (set >= descriptor_sets.size())
				break;

			const auto &descriptor_set{descriptor_sets[set]};
			for (auto &[name, write_descriptor]: descriptor_set.writeDescriptorSets)
			{
				DescriptorDeclaration &descriptor_declaration{m_descriptorDeclarations[name]};
			}
		}
	}

	void VKDescriptorSetManager::setDescriptor(const String &p_name, const RefPtr<VKUniformBuffer> &p_uniform_buffer)
	{
	}

	void VKDescriptorSetManager::setDescriptor(const String &p_name, const RefPtr<VKUniformBufferPFF> &p_uniform_buffer_pff)
	{
	}

	void VKDescriptorSetManager::bakeDescriptors()
	{
	}

	const std::vector<vk::raii::DescriptorSet> &VKDescriptorSetManager::getDescriptorSets(uint32 p_frame_index) const
	{
		TST_ASSERT_MSG(p_frame_index < VKGPUContext::c_maxFramesInFlight, "Frame index out of bounds");
		return {};
	}
}
