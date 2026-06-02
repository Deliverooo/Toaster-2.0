#include "toast_render/compute_pass.hpp"
#include "toast_render/render_context.hpp"

namespace toaster::render
{
	ComputePass::ComputePass(RenderContext &p_render_ctx, const gpu::ComputePipelineHandle &p_pipeline) : m_renderCtx(&p_render_ctx), m_pipeline(p_pipeline)
	{
		DescriptorSetManagerSpecInfo dsm_spec_info{};
		dsm_spec_info.shader   = m_pipeline->getShader();
		dsm_spec_info.startSet = 1u;
		dsm_spec_info.endSet   = 3u;
		m_descriptorSetManager = new DescriptorSetManager{*m_renderCtx, dsm_spec_info};
	}

	ComputePass::~ComputePass()
	{
		m_renderCtx->getLogicalDevice()->deferDestruction([dsm = m_descriptorSetManager]() mutable -> void { delete dsm; });
	}

	auto ComputePass::bake() -> void
	{
		m_descriptorSetManager->bakeDescriptors();
	}

	auto ComputePass::update(uint32 p_frame_index) -> void
	{
		m_descriptorSetManager->updateDescriptors(p_frame_index);
	}

	auto ComputePass::getPipeline() const -> const gpu::ComputePipelineHandle &
	{
		return m_pipeline;
	}

	auto ComputePass::getDescriptorSets(uint32 p_frame_index) const -> std::vector<vk::DescriptorSet>
	{
		return m_descriptorSetManager->getDescriptorSets(p_frame_index);
	}

	auto ComputePass::getStartSetIndex() const -> uint32
	{
		return m_descriptorSetManager->getSpecInfo().startSet;
	}

	auto ComputePass::getEndSetIndex() const -> uint32
	{
		return m_descriptorSetManager->getSpecInfo().endSet;
	}
}
