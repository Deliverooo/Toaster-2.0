#include "vk_descriptor_set_manager.hpp"
#include "vk_gpu_context.hpp"

namespace toaster::gpu
{
	VKDescriptorSetManager::VKDescriptorSetManager(VKGPUContext *p_ctx, const RefPtr<VKShader> &p_shader, uint32 p_start_set,
												   uint32        p_end_set) : m_ctx(p_ctx), m_shader(p_shader)
	{
		const auto &shader_descriptor_sets = m_shader->getReflectedShaderDescriptorSets();
		m_writeDescriptorMap.fill({});

		for (uint32 set{p_start_set}; set <= p_end_set; ++set)
		{
			if (set >= shader_descriptor_sets.size())
				break;

			const DescriptorSet &shader_descriptor = shader_descriptor_sets[set];
			for (auto &[name, write_descriptor] : shader_descriptor.writeDescriptorSets)
			{
				uint32 binding = write_descriptor.dstBinding;

			}
		}
	}

	void VKDescriptorSetManager::set(const String &p_name, const RefPtr<VKUniformBuffer> &p_uniform_buffer)
	{
	}

	void VKDescriptorSetManager::set(const String &p_name, const RefPtr<VKTexture2D> &p_texture_2d)
	{
	}
}
