#include "toast_render/render_pass.hpp"
#include "toast_render/render_context.hpp"

namespace toaster::render
{
	RenderPass::RenderPass(RenderContext &p_render_ctx, const gpu::PipelineHandle &p_pipeline) : m_renderCtx(&p_render_ctx), m_pipeline(p_pipeline)
	{
		DescriptorSetManagerSpecInfo dsm_spec_info{};
		dsm_spec_info.shader   = m_pipeline->getSpecInfo().shader;
		dsm_spec_info.startSet = 1u;
		dsm_spec_info.endSet   = 3u;
		m_descriptorSetManager = new DescriptorSetManager{*m_renderCtx, dsm_spec_info};
	}

	RenderPass::~RenderPass()
	{
		m_renderCtx->getLogicalDevice()->deferDestruction([dsm = m_descriptorSetManager]() mutable -> void { delete dsm; });
	}

	auto RenderPass::bake() -> void
	{
		m_descriptorSetManager->bakeDescriptors();
	}

	auto RenderPass::update(uint32 p_frame_index) -> void
	{
		m_descriptorSetManager->updateDescriptors(p_frame_index);
	}

	auto RenderPass::getPipeline() const -> const gpu::PipelineHandle &
	{
		return m_pipeline;
	}

	auto RenderPass::getDescriptorSets(uint32 p_frame_index) const -> std::vector<vk::DescriptorSet>
	{
		return m_descriptorSetManager->getDescriptorSets(p_frame_index);
	}

	auto RenderPass::getStartSetIndex() const -> uint32
	{
		return m_descriptorSetManager->getSpecInfo().startSet;
	}

	auto RenderPass::getEndSetIndex() const -> uint32
	{
		return m_descriptorSetManager->getSpecInfo().endSet;
	}
}
