#include "toast_gpu/vk/vk_render_pass.hpp"
#include "toast_gpu/vk/vk_logical_device.hpp"

namespace toaster::gpu
{
	VKRenderPass::VKRenderPass(VKLogicalDevice *p_device, const PipelineHandle &p_pipeline) : m_device(p_device), m_pipeline(p_pipeline)
	{
		TST_PERMA_ASSERT_MSG(p_device, "Device cannot be null");

		DescriptorSetManagerSpecInfo dsm_spec_info{};
		dsm_spec_info.shader   = m_pipeline->getSpecInfo().shader;
		dsm_spec_info.startSet = 1;
		dsm_spec_info.endSet   = 3;
		m_descriptorSetManager = new VKDescriptorSetManager{m_device, dsm_spec_info};
	}

	VKRenderPass::~VKRenderPass()
	{
		m_device->deferDestruction([dsm = m_descriptorSetManager]()mutable -> void
		{
			delete dsm;
		});
	}

	auto VKRenderPass::setInput(const String &p_name, const UniformBufferHandle &p_uniform_buffer) -> void
	{
		m_descriptorSetManager->setDescriptor(p_name, p_uniform_buffer);
	}

	auto VKRenderPass::setInput(const String &p_name, const UniformBufferPFFHandle &p_uniform_buffer_pff) -> void
	{
		m_descriptorSetManager->setDescriptor(p_name, p_uniform_buffer_pff);
	}

	auto VKRenderPass::setInput(const String &p_name, const Texture2DHandle &p_texture_2d) -> void
	{
		m_descriptorSetManager->setDescriptor(p_name, p_texture_2d);
	}

	auto VKRenderPass::setInput(const String &p_name, const StorageImageHandle &p_image_2d) -> void
	{
		m_descriptorSetManager->setDescriptor(p_name, p_image_2d);
	}

	auto VKRenderPass::setInput(const String &p_name, const Texture3DHandle &p_texture_3d) -> void
	{
		m_descriptorSetManager->setDescriptor(p_name, p_texture_3d);
	}

	auto VKRenderPass::bake() -> void
	{
		m_descriptorSetManager->bakeDescriptors();
	}

	auto VKRenderPass::update(uint32 p_frame_index) -> void
	{
		m_descriptorSetManager->updateDescriptors(p_frame_index);
	}

	auto VKRenderPass::getPipeline() const -> const PipelineHandle &
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
		return m_descriptorSetManager->getSpecInfo().startSet;
	}

	auto VKRenderPass::getEndSetIndex() const -> uint32
	{
		return m_descriptorSetManager->getSpecInfo().endSet;
	}
}
