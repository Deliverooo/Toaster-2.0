#include "vk_compute_pipeline.hpp"

#include "vk_gpu_context.hpp"

namespace toaster::gpu
{
	VKComputePipeline::VKComputePipeline(VKGPUContext *p_ctx, const RefPtr<VKShader> &p_shader) : m_ctx(p_ctx), m_shader(p_shader)
	{
		auto descriptor_set_layouts = m_shader->getDescriptorSetLayouts();

		auto &push_constant_ranges = m_shader->getReflectedPushConstantRanges();

		std::vector<vk::PushConstantRange> vk_push_constant_ranges{};
		for (auto &push_constant_range: push_constant_ranges)
		{
			auto &pcr{vk_push_constant_ranges.emplace_back()};
			pcr.size       = push_constant_range.size;
			pcr.offset     = push_constant_range.offset;
			pcr.stageFlags = push_constant_range.stage;
			LOG_TRACE("PCR: Size: {} | Offset: {}", pcr.size, pcr.offset);
		}
		vk::PipelineLayoutCreateInfo pipeline_layout_create_info{};
		pipeline_layout_create_info.pushConstantRangeCount = vk_push_constant_ranges.size();
		pipeline_layout_create_info.pPushConstantRanges    = vk_push_constant_ranges.data();
		pipeline_layout_create_info.setLayoutCount         = descriptor_set_layouts.size();
		pipeline_layout_create_info.pSetLayouts            = descriptor_set_layouts.data();
		m_pipelineLayout                                   = {m_ctx->getLogicalDevice()->getVulkanLogicalDevice(), pipeline_layout_create_info};

		const std::vector<vk::PipelineShaderStageCreateInfo> stage_infos = m_shader->getPipelineShaderStageCreateInfos();
		vk::ComputePipelineCreateInfo                        compute_pipeline_create_info{};
		compute_pipeline_create_info.layout = m_pipelineLayout;
		compute_pipeline_create_info.stage  = stage_infos[0];

		m_pipeline = {m_ctx->getLogicalDevice()->getVulkanLogicalDevice(), nullptr, compute_pipeline_create_info};
	}

	auto VKComputePipeline::getContext() const -> VKGPUContext *
	{
		return m_ctx;
	}

	auto VKComputePipeline::getShader() const -> const RefPtr<VKShader> &
	{
		return m_shader;
	}

	auto VKComputePipeline::getPipeline() -> vk::raii::Pipeline &
	{
		return m_pipeline;
	}

	auto VKComputePipeline::getPipelineLayout() -> vk::raii::PipelineLayout &
	{
		return m_pipelineLayout;
	}
}
