#include "toast_gpu/vk/vk_compute_pass.hpp"
#include "toast_gpu/vk/vk_logical_device.hpp"

namespace toaster::gpu
{
	VKComputePass::VKComputePass(VKLogicalDevice *p_device, const RefPtr<VKComputePipeline> &p_pipeline) : m_device(p_device), m_pipeline(p_pipeline)
	{
		TST_PERMA_ASSERT_MSG(p_device, "Device cannot be null");

		DescriptorSetManagerSpecInfo dsm_spec_info{};
		dsm_spec_info.shader   = m_pipeline->getShader();
		dsm_spec_info.startSet = 1;
		dsm_spec_info.endSet   = 3;
		m_descriptorSetManager = new VKDescriptorSetManager{m_device, dsm_spec_info};
	}

	VKComputePass::~VKComputePass()
	{
		m_device->deferDestruction([dsm = m_descriptorSetManager]()mutable -> void
		{
			delete dsm;
		});
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
		TST_ASSERT_MSG(p_frame_index < m_device->getSpecInfo().maxFramesInFlight, "Frame index out of bounds");
		return m_descriptorSetManager->getDescriptorSets(p_frame_index);
	}

	auto VKComputePass::getStartSetIndex() const -> uint32
	{
		return m_descriptorSetManager->getSpecInfo().startSet;
	}

	auto VKComputePass::getEndSetIndex() const -> uint32
	{
		return m_descriptorSetManager->getSpecInfo().endSet;
	}
}
