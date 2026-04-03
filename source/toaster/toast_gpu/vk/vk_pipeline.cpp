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
		vk::VertexInputBindingDescription      vertex_input_binding_description{};
		vertex_input_binding_description.binding                     = 0;
		vertex_input_binding_description.stride                      = m_createInfo.vertexBufferLayout.getStride();
		vertex_input_binding_description.inputRate                   = vk::VertexInputRate::eVertex;
		vertex_input_state_create_info.pVertexBindingDescriptions    = &vertex_input_binding_description;
		vertex_input_state_create_info.vertexBindingDescriptionCount = 1;

		std::vector<vk::VertexInputAttributeDescription> vertex_input_attribute_descriptions;
		vertex_input_attribute_descriptions.resize(m_createInfo.vertexBufferLayout.getElements().size());

		uint32 location{0u};
		for (const auto &element: m_createInfo.vertexBufferLayout)
		{
			vertex_input_attribute_descriptions[location].binding  = 0;
			vertex_input_attribute_descriptions[location].format   = _getVulkanAttribType(element.type);
			vertex_input_attribute_descriptions[location].location = location;
			vertex_input_attribute_descriptions[location].offset   = element.offset;
			++location;
		}

		vertex_input_state_create_info.pVertexAttributeDescriptions    = vertex_input_attribute_descriptions.data();
		vertex_input_state_create_info.vertexAttributeDescriptionCount = static_cast<uint32>(vertex_input_attribute_descriptions.size());

		vk::PipelineInputAssemblyStateCreateInfo input_assembly_state_create_info{};
		input_assembly_state_create_info.topology = vk::PrimitiveTopology::eTriangleList;

		vk::PipelineViewportStateCreateInfo viewport_state_create_info{};
		viewport_state_create_info.viewportCount = 1;
		viewport_state_create_info.scissorCount  = 1;

		vk::PipelineRasterizationStateCreateInfo rasterization_state_create_info{};
		rasterization_state_create_info.depthClampEnable        = false;
		rasterization_state_create_info.rasterizerDiscardEnable = false;
		rasterization_state_create_info.polygonMode             = vk::PolygonMode::eLine;
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

	vk::Format VKPipeline::_getVulkanAttribType(EShaderDataType p_type)
	{
		switch (p_type)
		{
			case EShaderDataType::eFloat: return vk::Format::eR32Sfloat;
			case EShaderDataType::eFloat2: return vk::Format::eR32G32Sfloat;
			case EShaderDataType::eFloat3: return vk::Format::eR32G32B32Sfloat;
			case EShaderDataType::eFloat4: return vk::Format::eR32G32B32A32Sfloat;
			case EShaderDataType::eInt: return vk::Format::eR32Sint;
			case EShaderDataType::eInt2: return vk::Format::eR32G32Sint;
			case EShaderDataType::eInt3: return vk::Format::eR32G32B32Sint;
			case EShaderDataType::eInt4: return vk::Format::eR32G32B32A32Sint;
			case EShaderDataType::eBool: return vk::Format::eR32Sint;
			default: return vk::Format::eUndefined;
		}
		TST_ASSERT_MSG(false, "Unsupported shader data type");
		return vk::Format::eUndefined;
	}
}
