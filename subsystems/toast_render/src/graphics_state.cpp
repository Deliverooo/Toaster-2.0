#include "toast_render/graphics_state.hpp"

#include "toast_render/render_context.hpp"

#include <ranges>

namespace toaster::render
{
	GraphicsState::GraphicsState(RenderContext &p_render_ctx) : m_renderCtx(&p_render_ctx)
	{
	}

	auto GraphicsState::bind(gpu::VKCommandBuffer *p_command_buffer) const -> void
	{
		auto &cmd{p_command_buffer ? p_command_buffer->getVulkanCommandBuffer() : m_renderCtx->getCurrentSwapchainCommandBuffer()->getVulkanCommandBuffer()};

		// Apparently, if certain shader features are enabled, you have to specify all shader stages even if they are unused...
		std::unordered_map<vk::ShaderStageFlagBits, vk::ShaderEXT> shader_stages_map{
			{vk::ShaderStageFlagBits::eVertex, nullptr},
			// {vk::ShaderStageFlagBits::eTessellationControl, nullptr},
			// {vk::ShaderStageFlagBits::eTessellationEvaluation, nullptr},
			// {vk::ShaderStageFlagBits::eGeometry, nullptr},
			{vk::ShaderStageFlagBits::eTaskEXT, nullptr},
			{vk::ShaderStageFlagBits::eMeshEXT, nullptr},
			{vk::ShaderStageFlagBits::eFragment, nullptr}
		};

		for (const auto &shader: m_shaders)
			shader_stages_map[shader->getStage()] = shader->getShader();

		const auto shader_stages{shader_stages_map | std::views::keys | std::ranges::to<std::vector>()};
		const auto shaders{shader_stages_map | std::views::values | std::ranges::to<std::vector>()};
		cmd.bindShadersEXT(shader_stages, shaders);

		// set the vertex input state
		cmd.setVertexInputEXT(m_vertexBufferLayout.getBindingDescription(), m_vertexBufferLayout.getAttributeDescriptions(0));

		// set the input assembly state
		cmd.setPrimitiveTopologyEXT(vk::PrimitiveTopology::eTriangleList);
		cmd.setPrimitiveRestartEnableEXT(false);

		// set the rasterization state
		cmd.setDepthClampEnableEXT(false);
		cmd.setDepthBiasEnableEXT(false);
		cmd.setRasterizerDiscardEnableEXT(false);
		cmd.setPolygonModeEXT(vk::PolygonMode::eFill);
		cmd.setCullModeEXT(m_cullMode);
		cmd.setFrontFaceEXT(vk::FrontFace::eCounterClockwise);
		cmd.setLineWidth(1.0f);

		// set the colour blend attachment state
		const auto colour_blend_enables{m_colourBlendAttachmentInfos | std::views::transform(&ColourBlendAttachmentInfo::blendEnable) | std::ranges::to<std::vector>()};
		const auto blend_equations{m_colourBlendAttachmentInfos | std::views::transform(&ColourBlendAttachmentInfo::blendEquation) | std::ranges::to<std::vector>()};
		const auto colour_write_masks{m_colourBlendAttachmentInfos | std::views::transform(&ColourBlendAttachmentInfo::colourWriteMask) | std::ranges::to<std::vector>()};

		cmd.setColorBlendEnableEXT(0, colour_blend_enables);
		cmd.setColorBlendEquationEXT(0, blend_equations);
		cmd.setColorWriteMaskEXT(0, colour_write_masks);

		// set depth stencil state
		cmd.setDepthTestEnableEXT(m_depthTestEnable);
		cmd.setDepthWriteEnableEXT(m_depthWriteEnable);
		cmd.setDepthCompareOpEXT(m_depthCompareOp);
		cmd.setStencilTestEnableEXT(false);

		// set multisample state
		if (m_enableMultisample)
		{
			auto sample_count{m_renderCtx->getPhysicalDevice()->getMaxUsableSampleCount()};
			cmd.setSampleMaskEXT(sample_count, 0xFFFFFFFF);
			cmd.setRasterizationSamplesEXT(sample_count);
		}
		else
		{
			cmd.setSampleMaskEXT(vk::SampleCountFlagBits::e1, 0xFFFFFFFF);
			cmd.setRasterizationSamplesEXT(vk::SampleCountFlagBits::e1);
		}

		cmd.setAlphaToCoverageEnableEXT(false);
	}
}
