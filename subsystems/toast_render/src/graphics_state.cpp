#include "toast_render/graphics_state.hpp"

#include "toast_render/render_context.hpp"

namespace toaster::render
{
	GraphicsState::GraphicsState(RenderContext &p_render_ctx, std::vector<gpu::DynamicShaderHandle> p_shaders) : m_renderCtx(&p_render_ctx), m_shaders(p_shaders)
	{
	}

	auto GraphicsState::bind(gpu::VKCommandBuffer *p_command_buffer) const -> void
	{
		auto &cmd{p_command_buffer ? p_command_buffer->getVulkanCommandBuffer() : m_renderCtx->getCurrentSwapchainCommandBuffer()->getVulkanCommandBuffer()};

		std::vector<vk::ShaderEXT>           shaders;
		std::vector<vk::ShaderStageFlagBits> stages;
		for (const auto &shader: m_shaders)
		{
			shaders.emplace_back(shader->getShader());
			stages.emplace_back(shader->getStage());
		}
		cmd.bindShadersEXT(stages, shaders);

		cmd.setPrimitiveTopologyEXT(vk::PrimitiveTopology::eTriangleList);
		cmd.setPrimitiveRestartEnableEXT(false);
		cmd.setRasterizerDiscardEnableEXT(false);
		cmd.setPolygonModeEXT(vk::PolygonMode::eFill);
		cmd.setCullModeEXT(vk::CullModeFlagBits::eBack);
		cmd.setFrontFaceEXT(vk::FrontFace::eCounterClockwise);
		cmd.setDepthBiasEnableEXT(false);

		cmd.setVertexInputEXT(m_vertexBufferLayout.getBindingDescription(), m_vertexBufferLayout.getAttributeDescriptions(0));

		cmd.setColorBlendEnableEXT(0, {false});
		vk::ColorBlendEquationEXT colour_blend_equation{};
		cmd.setColorBlendEquationEXT(0, {colour_blend_equation});
		cmd.setColorWriteMaskEXT(0, {vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA});


		cmd.setAlphaToCoverageEnableEXT(false);

		cmd.setDepthTestEnableEXT(false);
		cmd.setDepthWriteEnableEXT(false);
		cmd.setStencilTestEnableEXT(false);

		cmd.setSampleMaskEXT(vk::SampleCountFlagBits::e1, 0xFFFFFFFFu);
		cmd.setRasterizationSamplesEXT(vk::SampleCountFlagBits::e1);
	}
}
