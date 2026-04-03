#include "vk_pipeline.hpp"

#include "toast_lib/io/filesystem.hpp"

namespace toaster::gpu
{
	VKPipeline::VKPipeline(VKGPUContext *p_ctx, const PipelineCreateInfo &p_create_info) : m_ctx(p_ctx), m_createInfo(p_create_info)
	{
		TST_ASSERT_MSG(p_ctx, "GPU Context is null");
		_createGraphicsPipeline();
	}

	VKPipeline::~VKPipeline()
	{
	}

	vk::raii::Pipeline &VKPipeline::getPipeline()
	{
		return m_graphicsPipeline;
	}

	vk::raii::PipelineLayout &VKPipeline::getPipelineLayout()
	{
		return m_pipelineLayout;
	}

	const PipelineCreateInfo &VKPipeline::getCreateInfo() const
	{
		return m_createInfo;
	}

	void VKPipeline::_createGraphicsPipeline()
	{
		vk::raii::ShaderModule vertex_shader_module = m_ctx->createShaderModule(io::filesystem::readBinary("shaders/test.vert.glsl.spv"));
		vk::raii::ShaderModule pixel_shader_module  = m_ctx->createShaderModule(io::filesystem::readBinary("shaders/test.pixel.glsl.spv"));

		vk::PipelineShaderStageCreateInfo vertex_shader_stage_create_info{};
		vertex_shader_stage_create_info.stage  = vk::ShaderStageFlagBits::eVertex;
		vertex_shader_stage_create_info.module = vertex_shader_module;
		vertex_shader_stage_create_info.pName  = "main";

		vk::PipelineShaderStageCreateInfo pixel_shader_stage_create_info{};
		pixel_shader_stage_create_info.stage  = vk::ShaderStageFlagBits::eFragment;
		pixel_shader_stage_create_info.module = pixel_shader_module;
		pixel_shader_stage_create_info.pName  = "main";

		vk::PipelineShaderStageCreateInfo shader_stage_create_infos[] = {vertex_shader_stage_create_info, pixel_shader_stage_create_info};

		vk::PipelineVertexInputStateCreateInfo vertex_input_state_create_info{};

		vk::PipelineInputAssemblyStateCreateInfo input_assembly_state_create_info{};
		input_assembly_state_create_info.topology = vk::PrimitiveTopology::eTriangleList;

		vk::PipelineViewportStateCreateInfo viewport_state_create_info{};
		viewport_state_create_info.viewportCount = 1;
		viewport_state_create_info.scissorCount  = 1;

		vk::PipelineRasterizationStateCreateInfo rasterization_state_create_info{};
		rasterization_state_create_info.depthClampEnable        = false;
		rasterization_state_create_info.rasterizerDiscardEnable = false;
		rasterization_state_create_info.polygonMode             = vk::PolygonMode::eFill;
		rasterization_state_create_info.cullMode                = vk::CullModeFlagBits::eBack;
		rasterization_state_create_info.frontFace               = vk::FrontFace::eClockwise;
		rasterization_state_create_info.depthBiasEnable         = false;
		rasterization_state_create_info.lineWidth               = 1.0f;

		vk::PipelineColorBlendAttachmentState colour_blend_attachment_state{};
		colour_blend_attachment_state.blendEnable    = false;
		colour_blend_attachment_state.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
													   vk::ColorComponentFlagBits::eA;

		vk::PipelineColorBlendStateCreateInfo colour_blend_state_create_info{};
		colour_blend_state_create_info.logicOpEnable   = false;
		colour_blend_state_create_info.logicOp         = vk::LogicOp::eCopy;
		colour_blend_state_create_info.attachmentCount = 1;
		colour_blend_state_create_info.pAttachments    = &colour_blend_attachment_state;

		std::array                         dynamic_states{vk::DynamicState::eViewport, vk::DynamicState::eScissor};
		vk::PipelineDynamicStateCreateInfo dynamic_state_create_info{};
		dynamic_state_create_info.pDynamicStates    = dynamic_states.data();
		dynamic_state_create_info.dynamicStateCount = dynamic_states.size();

		vk::PipelineMultisampleStateCreateInfo multisample_state_create_info{};
		multisample_state_create_info.rasterizationSamples = vk::SampleCountFlagBits::e1;
		multisample_state_create_info.sampleShadingEnable  = false;

		vk::PipelineRenderingCreateInfo rendering_create_info{};
		rendering_create_info.colorAttachmentCount    = 1;
		rendering_create_info.pColorAttachmentFormats = &m_createInfo.colourAttachmentFormat;

		vk::PipelineLayoutCreateInfo pipeline_layout_create_info{};
		pipeline_layout_create_info.setLayoutCount         = 0;
		pipeline_layout_create_info.pushConstantRangeCount = 0;
		m_pipelineLayout                                   = {m_ctx->getDevice(), pipeline_layout_create_info};

		vk::GraphicsPipelineCreateInfo graphics_pipeline_create_info{};
		graphics_pipeline_create_info.stageCount          = 2;
		graphics_pipeline_create_info.pStages             = shader_stage_create_infos;
		graphics_pipeline_create_info.pVertexInputState   = &vertex_input_state_create_info;
		graphics_pipeline_create_info.pInputAssemblyState = &input_assembly_state_create_info;
		graphics_pipeline_create_info.pRasterizationState = &rasterization_state_create_info;
		graphics_pipeline_create_info.pViewportState      = &viewport_state_create_info;
		graphics_pipeline_create_info.pMultisampleState   = &multisample_state_create_info;
		graphics_pipeline_create_info.pColorBlendState    = &colour_blend_state_create_info;
		graphics_pipeline_create_info.pDynamicState       = &dynamic_state_create_info;
		graphics_pipeline_create_info.layout              = m_pipelineLayout;
		graphics_pipeline_create_info.renderPass          = nullptr;
		graphics_pipeline_create_info.pNext               = &rendering_create_info;

		m_graphicsPipeline = {m_ctx->getDevice(), nullptr, graphics_pipeline_create_info};
	}
}
