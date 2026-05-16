#include "vk_compute_pass.hpp"
#include "vk_logical_device.hpp"

namespace toaster::gpu
{
	VKComputePass::VKComputePass(VKLogicalDevice *p_device, const RefPtr<VKComputePipeline> &p_pipeline) : m_device(p_device), m_pipeline(p_pipeline)
	{
		m_descriptorSetManager = new VKDescriptorSetManager{m_device, m_pipeline->getShader(), 1, 3};
	}

	VKComputePass::~VKComputePass()
	{
		m_device->deferDestruction([dsm = m_descriptorSetManager]()mutable -> void
		{
			delete dsm;
		});
	}

	auto VKComputePass::setInput(const String &p_name, RefPtr<VKUniformBuffer> &p_uniform_buffer) -> void
	{
		m_descriptorSetManager->setDescriptor(p_name, p_uniform_buffer);
	}

	auto VKComputePass::setInput(const String &p_name, RefPtr<VKUniformBufferPFF> &p_uniform_buffer_pff) -> void
	{
		m_descriptorSetManager->setDescriptor(p_name, p_uniform_buffer_pff);
	}

	auto VKComputePass::setInput(const String &p_name, RefPtr<VKStorageBuffer> &p_storage_buffer) -> void
	{
		m_descriptorSetManager->setDescriptor(p_name, p_storage_buffer);
	}

	auto VKComputePass::setInput(const String &p_name, RefPtr<VKStorageBufferPFF> &p_storage_buffer_pff) -> void
	{
		m_descriptorSetManager->setDescriptor(p_name, p_storage_buffer_pff);
	}

	auto VKComputePass::setInput(const String &p_name, RefPtr<VKTexture2D> &p_texture_2d) -> void
	{
		m_descriptorSetManager->setDescriptor(p_name, p_texture_2d);
	}

	auto VKComputePass::setInput(const String &p_name, RefPtr<VKStorageImage> &p_image_2d) -> void
	{
		m_descriptorSetManager->setDescriptor(p_name, p_image_2d);
	}

	auto VKComputePass::setInput(const String &p_name, RefPtr<VKTexture3D> &p_texture_3d) -> void
	{
		m_descriptorSetManager->setDescriptor(p_name, p_texture_3d);
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
		return m_descriptorSetManager->getStartSetIndex();
	}

	auto VKComputePass::getEndSetIndex() const -> uint32
	{
		return m_descriptorSetManager->getEndSetIndex();
	}
}
