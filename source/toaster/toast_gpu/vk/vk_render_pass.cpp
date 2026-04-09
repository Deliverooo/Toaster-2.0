#include "vk_render_pass.hpp"

#include "vk_gpu_context.hpp"

namespace toaster::gpu
{
	VKRenderPass::VKRenderPass(VKGPUContext *p_ctx, const RefPtr<VKPipeline> &p_pipeline) : m_ctx(p_ctx), m_pipeline(p_pipeline)
	{
		m_descriptorSetManager = make_unique<VKDescriptorSetManager>(m_ctx, m_pipeline->getCreateInfo().shader, 1, 3);
	}

	VKGPUContext *VKRenderPass::getContext() const
	{
		return m_ctx;
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

	void VKRenderPass::update(uint32 p_frame_index)
	{
		m_descriptorSetManager->updateDescriptors(p_frame_index);
	}

	const RefPtr<VKPipeline> &VKRenderPass::getPipeline() const
	{
		return m_pipeline;
	}

	std::vector<vk::DescriptorSet> VKRenderPass::getDescriptorSets(uint32 p_frame_index) const
	{
		TST_ASSERT_MSG(p_frame_index < VKGPUContext::c_maxFramesInFlight, "Frame index out of bounds");
		return m_descriptorSetManager->getDescriptorSets(p_frame_index);
	}

	uint32 VKRenderPass::getStartSetIndex() const
	{
		return m_descriptorSetManager->getStartSetIndex();
	}

	uint32 VKRenderPass::getEndSetIndex() const
	{
		return m_descriptorSetManager->getEndSetIndex();
	}
}
