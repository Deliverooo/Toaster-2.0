#include "toast_render/skybox_pass.hpp"

#include "toast_render/globals.hpp"
#include "toast_render/render_context.hpp"

namespace toaster::render
{
	SkyboxPass::SkyboxPass(RenderContext &p_render_ctx) : m_renderCtx(&p_render_ctx)
	{
		_construct();
		m_environmentMap = m_renderCtx->getGlobals()->whiteImage();
	}

	SkyboxPass::SkyboxPass(RenderContext &p_render_ctx, const ImageHandle &p_environment_map) : m_renderCtx(&p_render_ctx), m_environmentMap(p_environment_map)
	{
		_construct();
	}

	auto SkyboxPass::onRender(gpu::CommandBuffer &p_cmd, const RenderingInfo &p_rendering_info, uintptr p_camera_bda_ptr) const -> void
	{
		p_cmd.bindShaders({m_renderCtx->getGlobals()->getShader("Skybox_VS"), m_renderCtx->getGlobals()->getShader("Skybox_PS")});

		p_cmd.setPrimitiveTopology(gpu::EPrimitiveTopology::eTriangleList);
		p_cmd.setCullMode(gpu::ECullMode::eNone);
		p_cmd.setFrontFace(gpu::EFrontFace::eCCW);

		p_cmd.setPolygonMode(gpu::EPolygonMode::eFill);

		p_cmd.setDepthTestEnable(false);
		p_cmd.setStencilTestEnable(false);

		p_cmd.getVulkanCommandBuffer().setRasterizerDiscardEnableEXT(false);


		p_cmd.getVulkanCommandBuffer().setColorBlendEnableEXT(0, {false});
		p_cmd.getVulkanCommandBuffer().setColorWriteMaskEXT(0, {{vk::FlagTraits<vk::ColorComponentFlagBits>::allFlags}});

		p_cmd.getVulkanCommandBuffer().setSampleMaskEXT(vk::SampleCountFlagBits::e1, 0xFFFFFFFF);
		p_cmd.getVulkanCommandBuffer().setRasterizationSamplesEXT(vk::SampleCountFlagBits::e1);

		m_renderCtx->beginRendering(p_rendering_info, &p_cmd);

		SkyboxConstants skybox_constants{};
		skybox_constants.vertexBufferBDA             = m_renderCtx->getGlobals()->fullscreenQuadVertexBuffer().getDeviceAddress();
		skybox_constants.cameraBDA                   = p_camera_bda_ptr;
		skybox_constants.samplerAddressOffset        = m_renderCtx->getSampler(ESamplerType::eIrradianceMap);
		skybox_constants.environmentMapAddressOffset = m_environmentMap->getAlignedShaderReadHeapID();
		p_cmd.pushData(skybox_constants);

		m_renderCtx->renderFullscreenQuad();
		m_renderCtx->endRendering(p_rendering_info, &p_cmd);
	}

	auto SkyboxPass::setEnvironmentMap(const ImageHandle &p_environment_map) -> void
	{
		m_environmentMap = p_environment_map;
	}

	auto SkyboxPass::_construct() -> void
	{
		m_skyboxState = m_renderCtx->createUnique<GraphicsState>();
		m_skyboxState->setShaders({m_renderCtx->getGlobals()->getShader("Skybox_VS"), m_renderCtx->getGlobals()->getShader("Skybox_PS")}).setAttachmentCount(1u).
				setCullMode(vk::CullModeFlagBits::eNone).setVertexBufferLayout(RenderContext::fullscreenQuadVbl).setEnableDepthTest(false).setEnableDepthWrite(false).
				setEnableMultisample(false);
	}
}
