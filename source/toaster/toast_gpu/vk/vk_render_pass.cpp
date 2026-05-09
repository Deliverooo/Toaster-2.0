#include "vk_render_pass.hpp"

#include "vk_logical_device.hpp"

namespace toaster::gpu
{
	VKRenderPass::VKRenderPass(VKLogicalDevice *p_device, const RefPtr<VKPipeline> &p_pipeline) : m_device(p_device), m_pipeline(p_pipeline)
	{
		m_descriptorSetManager = make_unique<VKDescriptorSetManager>(m_device, m_pipeline->getSpecInfo().shader, 1, 3);
	}

	auto VKRenderPass::setInput(const String &p_name, const RefPtr<VKUniformBuffer> &p_uniform_buffer) -> void
	{
		m_descriptorSetManager->setDescriptor(p_name, p_uniform_buffer);
	}

	auto VKRenderPass::setInput(const String &p_name, const RefPtr<VKUniformBufferPFF> &p_uniform_buffer_pff) -> void
	{
		m_descriptorSetManager->setDescriptor(p_name, p_uniform_buffer_pff);
	}

	auto VKRenderPass::setInput(const String &p_name, const RefPtr<VKTexture2D> &p_texture_2d) -> void
	{
		m_descriptorSetManager->setDescriptor(p_name, p_texture_2d);
	}

	auto VKRenderPass::setInput(const String &p_name, const RefPtr<VKImage2D> &p_image_2d) -> void
	{
		m_descriptorSetManager->setDescriptor(p_name, p_image_2d);
	}

	auto VKRenderPass::bake() -> void
	{
		m_descriptorSetManager->bakeDescriptors();
	}

	auto VKRenderPass::update(uint32 p_frame_index) -> void
	{
		m_descriptorSetManager->updateDescriptors(p_frame_index);
	}

	auto VKRenderPass::getPipeline() const -> const RefPtr<VKPipeline> &
	{
		return m_pipeline;
	}

	auto VKRenderPass::getDescriptorSets(uint32 p_frame_index) const -> std::vector<vk::DescriptorSet>
	{
		TST_ASSERT_MSG(p_frame_index < m_device->getSpecInfo().maxFramesInFlight, "Frame index out of bounds");
		return m_descriptorSetManager->getDescriptorSets(p_frame_index);
	}

	auto VKRenderPass::getStartSetIndex() const -> uint32
	{
		return m_descriptorSetManager->getStartSetIndex();
	}

	auto VKRenderPass::getEndSetIndex() const -> uint32
	{
		return m_descriptorSetManager->getEndSetIndex();
	}
}
