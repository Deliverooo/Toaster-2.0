#include "vk_render_pass.hpp"

#include "vk_gpu_context.hpp"

namespace toaster::gpu
{
	VKRenderPass::VKRenderPass(VKGPUContext *p_ctx, const RefPtr<VKPipeline> &p_pipeline) : m_ctx(p_ctx), m_pipeline(p_pipeline)
	{
		m_descriptorSetManager = make_unique<VKDescriptorSetManager>(m_ctx, m_pipeline->getCreateInfo().shader, 0, 3);
	}

	void VKRenderPass::setInput(const String &p_name, const RefPtr<VKUniformBuffer> &p_uniform_buffer)
	{
		m_descriptorSetManager->setDescriptor(p_name, p_uniform_buffer);
	}

	void VKRenderPass::setInput(const String &p_name, const RefPtr<VKUniformBufferPFF> &p_uniform_buffer_pff)
	{
		m_descriptorSetManager->setDescriptor(p_name, p_uniform_buffer_pff);
	}

	void VKRenderPass::setInput(const String &p_name, const RefPtr<VKTexture2D> &p_texture_2d)
	{
		m_descriptorSetManager->setDescriptor(p_name, p_texture_2d);
	}

	void VKRenderPass::bake()
	{
		m_descriptorSetManager->bakeDescriptors();
	}

	std::vector<vk::DescriptorSet> VKRenderPass::getDescriptorSets(uint32 p_frame_index) const
	{
		TST_ASSERT_MSG(p_frame_index < VKGPUContext::c_maxFramesInFlight, "Frame index out of bounds");
		return m_descriptorSetManager->getDescriptorSets(p_frame_index);
	}

	const RefPtr<VKPipeline> &VKRenderPass::getPipeline() const
	{
		return m_pipeline;
	}
}
