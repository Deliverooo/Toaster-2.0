#include "toast_gpu/vk/vk_pipeline.hpp"
#include "toast_gpu/vk/vk_logical_device.hpp"
#include "toast_lib/io/filesystem.hpp"

namespace toaster::gpu
{
	VKPipeline::VKPipeline(VKLogicalDevice *p_device, const PipelineSpecInfo &p_spec_info) : m_device(p_device), m_specInfo(p_spec_info)
	{
		TST_ASSERT_MSG(m_device, "Context cannot be null");

		_createGraphicsPipeline();
	}

	auto VKPipeline::getPipeline() -> vk::raii::Pipeline &
	{
		return m_graphicsPipeline;
	}

	auto VKPipeline::getPipelineLayout() -> const vk::raii::PipelineLayout &
	{
		return m_pipelineLayout;
	}

	auto VKPipeline::getSpecInfo() const -> const PipelineSpecInfo &
	{
		return m_specInfo;
	}

	auto VKPipeline::getVulkanAttribType(EBufferDataType p_type) -> vk::Format
	{
		switch (p_type)
		{
			case EBufferDataType::eFloat: return vk::Format::eR32Sfloat;
			case EBufferDataType::eFloat2: return vk::Format::eR32G32Sfloat;
			case EBufferDataType::eFloat3: return vk::Format::eR32G32B32Sfloat;
			case EBufferDataType::eFloat4: return vk::Format::eR32G32B32A32Sfloat;
			case EBufferDataType::eMat3: return vk::Format::eR32G32B32A32Sfloat; // TODO: If I ever want to do instanced rendering, I will need to look into ts
			case EBufferDataType::eMat4: return vk::Format::eR32G32B32A32Sfloat;
			case EBufferDataType::eInt: return vk::Format::eR32Sint;
			case EBufferDataType::eInt2: return vk::Format::eR32G32Sint;
			case EBufferDataType::eInt3: return vk::Format::eR32G32B32Sint;
			case EBufferDataType::eInt4: return vk::Format::eR32G32B32A32Sint;
			case EBufferDataType::eBool: return vk::Format::eR32Sint;
			default: return vk::Format::eUndefined;
		}
		TST_ASSERT_MSG(false, "Unsupported shader data type");
		return vk::Format::eUndefined;
	}

	auto VKPipeline::_createGraphicsPipeline() -> void
	{
		vk::PipelineVertexInputStateCreateInfo vertex_input_state_create_info{};

		std::vector<vk::VertexInputBindingDescription> vertex_input_binding_descriptions;

		{
			vk::VertexInputBindingDescription &vertex_input_binding_description{vertex_input_binding_descriptions.emplace_back()};
			vertex_input_binding_description.binding   = 0;
			vertex_input_binding_description.stride    = m_specInfo.vertexBufferLayout.getStride();
			vertex_input_binding_description.inputRate = vk::VertexInputRate::eVertex;
		}

		if (!m_specInfo.instanceLayout.getElements().empty())
		{
			vk::VertexInputBindingDescription &vertex_input_binding_description{vertex_input_binding_descriptions.emplace_back()};
			vertex_input_binding_description.binding   = 1;
			vertex_input_binding_description.stride    = m_specInfo.instanceLayout.getStride();
			vertex_input_binding_description.inputRate = vk::VertexInputRate::eInstance;
		}
		vertex_input_state_create_info.pVertexBindingDescriptions    = vertex_input_binding_descriptions.data();
		vertex_input_state_create_info.vertexBindingDescriptionCount = vertex_input_binding_descriptions.size();

		std::vector<vk::VertexInputAttributeDescription> vertex_input_attribute_descriptions;
		vertex_input_attribute_descriptions.resize(m_specInfo.vertexBufferLayout.getElements().size() + m_specInfo.instanceLayout.getElements().size());

		uint32 binding{0u};
		uint32 location{0u};
		for (const auto &layout: {m_specInfo.vertexBufferLayout, m_specInfo.instanceLayout})
		{
			for (const auto &element: layout)
			{
				vertex_input_attribute_descriptions[location].binding  = binding;
				vertex_input_attribute_descriptions[location].format   = getVulkanAttribType(element.type);
				vertex_input_attribute_descriptions[location].location = location;
				vertex_input_attribute_descriptions[location].offset   = element.offset;
				++location;
			}
			++binding;
		}
		vertex_input_state_create_info.pVertexAttributeDescriptions    = vertex_input_attribute_descriptions.data();
		vertex_input_state_create_info.vertexAttributeDescriptionCount = static_cast<uint32>(vertex_input_attribute_descriptions.size());

		vk::PipelineInputAssemblyStateCreateInfo input_assembly_state_create_info{};
		input_assembly_state_create_info.topology = m_specInfo.primitiveTopology;

		vk::PipelineViewportStateCreateInfo viewport_state_create_info{};
		viewport_state_create_info.viewportCount = 1;
		viewport_state_create_info.scissorCount  = 1;

		vk::PipelineRasterizationStateCreateInfo rasterization_state_create_info{};
		rasterization_state_create_info.depthClampEnable        = false;
		rasterization_state_create_info.rasterizerDiscardEnable = false;
		rasterization_state_create_info.polygonMode             = m_specInfo.polygonMode;
		rasterization_state_create_info.cullMode                = m_specInfo.cullMode;
		rasterization_state_create_info.frontFace               = vk::FrontFace::eCounterClockwise;
		rasterization_state_create_info.depthBiasEnable         = false;
		rasterization_state_create_info.lineWidth               = 1.0f;

		std::vector<vk::PipelineColorBlendAttachmentState> colour_blend_attachment_states{};
		for ([[maybe_unused]] vk::Format attachment: m_specInfo.colourAttachments)
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

		if (m_specInfo.multisample)
		{
			multisample_state_create_info.rasterizationSamples = m_device->getPhysicalDevice()->getMaxUsableSampleCount();
			multisample_state_create_info.sampleShadingEnable  = true;
		}
		else
		{
			multisample_state_create_info.rasterizationSamples = vk::SampleCountFlagBits::e1;
			multisample_state_create_info.sampleShadingEnable  = false;
		}

		vk::PipelineRenderingCreateInfo rendering_create_info{};
		if (!m_specInfo.colourAttachments.empty())
		{
			rendering_create_info.colorAttachmentCount    = m_specInfo.colourAttachments.size();
			rendering_create_info.pColorAttachmentFormats = m_specInfo.colourAttachments.data();
		}
		else
		{
			rendering_create_info.colorAttachmentCount    = 0u;
			rendering_create_info.pColorAttachmentFormats = nullptr;
		}
		rendering_create_info.depthAttachmentFormat = m_specInfo.depthFormat;

		vk::PipelineDepthStencilStateCreateInfo depth_stencil_state_create_info{};
		if (m_specInfo.depthFormat != vk::Format::eUndefined)
		{
			depth_stencil_state_create_info.depthTestEnable       = m_specInfo.depthTest;
			depth_stencil_state_create_info.depthWriteEnable      = m_specInfo.depthWrite;
			depth_stencil_state_create_info.depthCompareOp        = m_specInfo.depthCompare;
			depth_stencil_state_create_info.depthBoundsTestEnable = false;
			depth_stencil_state_create_info.stencilTestEnable     = false;
		}
		auto descriptor_set_layouts = m_specInfo.shader->getDescriptorSetLayouts();
		TST_ASSERT(!descriptor_set_layouts.empty());

		auto &push_constant_ranges = m_specInfo.shader->getReflectedPushConstantRanges();

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
		m_pipelineLayout                                   = {*m_device, pipeline_layout_create_info};

		std::vector<vk::PipelineShaderStageCreateInfo> stage_infos = m_specInfo.shader->getPipelineShaderStageCreateInfos();

		vk::GraphicsPipelineCreateInfo graphics_pipeline_create_info{};
		graphics_pipeline_create_info.stageCount          = stage_infos.size();
		graphics_pipeline_create_info.pStages             = stage_infos.data();
		graphics_pipeline_create_info.pVertexInputState   = &vertex_input_state_create_info;
		graphics_pipeline_create_info.pInputAssemblyState = &input_assembly_state_create_info;
		graphics_pipeline_create_info.pRasterizationState = &rasterization_state_create_info;
		graphics_pipeline_create_info.pViewportState      = &viewport_state_create_info;
		graphics_pipeline_create_info.pMultisampleState   = &multisample_state_create_info;
		graphics_pipeline_create_info.pColorBlendState    = m_specInfo.colourAttachments.empty() ? nullptr : &colour_blend_state_create_info;
		graphics_pipeline_create_info.pDynamicState       = &dynamic_state_create_info;
		graphics_pipeline_create_info.pDepthStencilState  = &depth_stencil_state_create_info;
		graphics_pipeline_create_info.layout              = m_pipelineLayout;
		graphics_pipeline_create_info.renderPass          = nullptr;
		graphics_pipeline_create_info.pNext               = &rendering_create_info;

		m_graphicsPipeline = {*m_device, nullptr, graphics_pipeline_create_info};
	}
}
