#include "vk_pipeline.hpp"

#include "toast_lib/io/filesystem.hpp"

#include "vk_gpu_context.hpp"
#include "toast_lib/logging.hpp"

namespace toaster::gpu
{
	VKPipeline::VKPipeline(VKGPUContext *p_ctx, const PipelineCreateInfo &p_create_info) : m_ctx(p_ctx), m_createInfo(p_create_info)
	{
		TST_ASSERT_MSG(p_ctx, "Context cannot be null");

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
		rasterization_state_create_info.polygonMode             = vk::PolygonMode::eFill;
		rasterization_state_create_info.cullMode                = vk::CullModeFlagBits::eNone;
		rasterization_state_create_info.frontFace               = vk::FrontFace::eCounterClockwise;
		rasterization_state_create_info.depthBiasEnable         = false;
		rasterization_state_create_info.lineWidth               = 1.0f;

		std::vector<vk::PipelineColorBlendAttachmentState> colour_blend_attachment_states{};
		for (vk::Format attachment: m_createInfo.colourAttachments)
		{
			auto &colour_blend_attachment_state{colour_blend_attachment_states.emplace_back()};
			colour_blend_attachment_state.blendEnable    = false;
			colour_blend_attachment_state.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
														   vk::ColorComponentFlagBits::eA;
		}

		vk::PipelineColorBlendStateCreateInfo colour_blend_state_create_info{};
		colour_blend_state_create_info.logicOpEnable   = false;
		colour_blend_state_create_info.logicOp         = vk::LogicOp::eCopy;
		colour_blend_state_create_info.attachmentCount = colour_blend_attachment_states.size();
		colour_blend_state_create_info.pAttachments    = colour_blend_attachment_states.data();

		std::array                         dynamic_states{vk::DynamicState::eViewport, vk::DynamicState::eScissor};
		vk::PipelineDynamicStateCreateInfo dynamic_state_create_info{};
		dynamic_state_create_info.pDynamicStates    = dynamic_states.data();
		dynamic_state_create_info.dynamicStateCount = dynamic_states.size();

		vk::PipelineMultisampleStateCreateInfo multisample_state_create_info{};

		if (m_createInfo.multisample)
		{
			multisample_state_create_info.rasterizationSamples = m_ctx->getMaxUsableSampleCount();
			multisample_state_create_info.sampleShadingEnable  = true;
		}
		else
		{
			multisample_state_create_info.rasterizationSamples = vk::SampleCountFlagBits::e1;
			multisample_state_create_info.sampleShadingEnable  = false;
		}

		TST_ASSERT(!m_createInfo.colourAttachments.empty());
		vk::PipelineRenderingCreateInfo rendering_create_info{};
		rendering_create_info.colorAttachmentCount    = m_createInfo.colourAttachments.size();
		rendering_create_info.pColorAttachmentFormats = m_createInfo.colourAttachments.data();
		rendering_create_info.depthAttachmentFormat   = m_ctx->findDepthFormat();

		vk::PipelineDepthStencilStateCreateInfo depth_stencil_state_create_info{};
		if (m_createInfo.depthFormat != vk::Format::eUndefined)
		{
			depth_stencil_state_create_info.depthTestEnable       = true;
			depth_stencil_state_create_info.depthWriteEnable      = true;
			depth_stencil_state_create_info.depthCompareOp        = vk::CompareOp::eLess;
			depth_stencil_state_create_info.depthBoundsTestEnable = false;
			depth_stencil_state_create_info.stencilTestEnable     = false;
		}
		auto descriptor_set_layouts = m_createInfo.shader->getDescriptorSetLayouts();
		TST_ASSERT(!descriptor_set_layouts.empty());

		auto &push_constant_ranges = m_createInfo.shader->getReflectedPushConstantRanges();

		std::vector<vk::PushConstantRange> vk_push_constant_ranges{};
		for (auto &push_constant_range: push_constant_ranges)
		{
			auto &pcr{vk_push_constant_ranges.emplace_back()};
			pcr.size       = push_constant_range.size;
			pcr.offset     = push_constant_range.offset;
			pcr.stageFlags = push_constant_range.stage;
		}

		vk::PipelineLayoutCreateInfo pipeline_layout_create_info{};
		pipeline_layout_create_info.pushConstantRangeCount = vk_push_constant_ranges.size();
		pipeline_layout_create_info.pPushConstantRanges    = vk_push_constant_ranges.data();
		pipeline_layout_create_info.setLayoutCount         = descriptor_set_layouts.size();
		pipeline_layout_create_info.pSetLayouts            = descriptor_set_layouts.data();
		m_pipelineLayout                                   = {m_ctx->getDevice(), pipeline_layout_create_info};

		std::vector<vk::PipelineShaderStageCreateInfo> stage_infos = m_createInfo.shader->getPipelineShaderStageCreateInfos();

		vk::GraphicsPipelineCreateInfo graphics_pipeline_create_info{};
		graphics_pipeline_create_info.stageCount          = stage_infos.size();
		graphics_pipeline_create_info.pStages             = stage_infos.data();
		graphics_pipeline_create_info.pVertexInputState   = &vertex_input_state_create_info;
		graphics_pipeline_create_info.pInputAssemblyState = &input_assembly_state_create_info;
		graphics_pipeline_create_info.pRasterizationState = &rasterization_state_create_info;
		graphics_pipeline_create_info.pViewportState      = &viewport_state_create_info;
		graphics_pipeline_create_info.pMultisampleState   = &multisample_state_create_info;
		graphics_pipeline_create_info.pColorBlendState    = &colour_blend_state_create_info;
		graphics_pipeline_create_info.pDynamicState       = &dynamic_state_create_info;
		graphics_pipeline_create_info.pDepthStencilState  = &depth_stencil_state_create_info;
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
