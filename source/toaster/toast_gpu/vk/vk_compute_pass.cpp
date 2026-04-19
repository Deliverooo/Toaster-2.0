#include "vk_compute_pass.hpp"
#include "vk_gpu_context.hpp"

namespace toaster::gpu
{
	VKComputePass::VKComputePass(VKGPUContext *p_ctx, const RefPtr<VKComputePipeline> &p_pipeline) : m_ctx(p_ctx), m_pipeline(p_pipeline)
	{
		m_descriptorSetManager = make_unique<VKDescriptorSetManager>(m_ctx, m_pipeline->getShader(), 1, 3);
	}

	auto VKComputePass::getContext() const -> VKGPUContext *
	{
		return m_ctx;
	}

	auto VKComputePass::setInput(const String &p_name, const RefPtr<VKUniformBuffer> &p_uniform_buffer) -> void
	{
		m_descriptorSetManager->setDescriptor(p_name, p_uniform_buffer);
	}

	auto VKComputePass::setInput(const String &p_name, const RefPtr<VKUniformBufferPFF> &p_uniform_buffer_pff) -> void
	{
		m_descriptorSetManager->setDescriptor(p_name, p_uniform_buffer_pff);
	}

	auto VKComputePass::setInput(const String &p_name, const RefPtr<VKStorageBuffer> &p_storage_buffer) -> void
	{
		m_descriptorSetManager->setDescriptor(p_name, p_storage_buffer);
	}

	auto VKComputePass::setInput(const String &p_name, const RefPtr<VKStorageBufferPFF> &p_storage_buffer_pff) -> void
	{
		m_descriptorSetManager->setDescriptor(p_name, p_storage_buffer_pff);
	}

	auto VKComputePass::setInput(const String &p_name, const RefPtr<VKTexture2D> &p_texture_2d) -> void
	{
		m_descriptorSetManager->setDescriptor(p_name, p_texture_2d);
	}

	auto VKComputePass::bake() -> void
	{
		m_descriptorSetManager->bakeDescriptors();
	}

	auto VKComputePass::update(uint32 p_frame_index) -> void
	{
		m_descriptorSetManager->updateDescriptors(p_frame_index);
	}

	auto VKComputePass::getPipeline() const -> const RefPtr<VKComputePipeline> &
	{
		return m_pipeline;
	}

	auto VKComputePass::getDescriptorSets(uint32 p_frame_index) const -> std::vector<vk::DescriptorSet>
	{
		TST_ASSERT_MSG(p_frame_index < VKGPUContext::c_maxFramesInFlight, "Frame index out of bounds");
		return m_descriptorSetManager->getDescriptorSets(p_frame_index);
	}

	auto VKComputePass::getStartSetIndex() const -> uint32
	{
		return m_descriptorSetManager->getStartSetIndex();
	}

	auto VKComputePass::getEndSetIndex() const -> uint32
	{
		return m_descriptorSetManager->getEndSetIndex();
	}
}
