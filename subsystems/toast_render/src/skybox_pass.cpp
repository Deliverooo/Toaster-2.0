#include "toast_render/skybox_pass.hpp"

#include "toast_render/globals.hpp"
#include "toast_render/render_context.hpp"

namespace toaster::render
{
	SkyboxPass::SkyboxPass(RenderContext &p_render_ctx, bool32 p_msaa) : m_renderCtx(&p_render_ctx)
	{
		_construct(p_msaa);
	}

	SkyboxPass::SkyboxPass(RenderContext &p_render_ctx, tsm::uint2 p_initial_viewport_size, bool32 p_msaa) : m_renderCtx(&p_render_ctx), m_viewportSize(p_initial_viewport_size)
	{
		_construct(p_msaa);
		m_environmentMap = m_renderCtx->getGlobals()->whiteImage();
	}

	SkyboxPass::SkyboxPass(RenderContext &p_render_ctx, tsm::uint2 p_initial_viewport_size, const ImageHandle &p_environment_map, bool32 p_msaa) : m_renderCtx(&p_render_ctx),
																																	m_viewportSize(p_initial_viewport_size),
																																	m_environmentMap(p_environment_map)
	{
		_construct(p_msaa);
	}

	auto SkyboxPass::getOutputImage() const -> const ImageHandle &
	{
		return m_renderTargetImage;
	}

	auto SkyboxPass::onRender(gpu::CommandBuffer &p_cmd, uintptr p_camera_bda_ptr) const -> void
	{
		RenderingInfo rendering_info{m_viewportSize};
		rendering_info.addColourAttachment(*m_renderTargetImage->getImage());

		onRender(p_cmd, p_camera_bda_ptr, rendering_info);
	}

	auto SkyboxPass::onRender(gpu::CommandBuffer &p_cmd, uintptr p_camera_bda_ptr, const RenderingInfo &p_rendering_info) const -> void
	{
		m_skyboxState->bind(&p_cmd);
		m_renderCtx->beginRendering(p_rendering_info, &p_cmd);

		SkyboxConstants skybox_constants{};
		skybox_constants.cameraBDA                   = p_camera_bda_ptr;
		skybox_constants.samplerAddressOffset        = m_renderCtx->getSampler(ESamplerType::eDefault);
		skybox_constants.environmentMapAddressOffset = m_environmentMap->getAlignedShaderReadHeapID();
		p_cmd.pushData(skybox_constants);

		m_renderCtx->renderFullscreenQuad();
		m_renderCtx->endRendering(p_rendering_info, &p_cmd);
	}

	auto SkyboxPass::onResize(tsm::uint2 p_size) -> void
	{
		TST_ASSERT_MSG(p_size.x != 0.0f || p_size.y != 0.0f, "Viewport size cannot be 0!");

		if (m_viewportSize.x == UINT32_MAX || m_viewportSize.y == UINT32_MAX) // Special value if using override rendering info
			return;

		m_viewportSize = p_size;

		if (m_renderTargetImage)
			m_renderTargetImage->resize(p_size);
	}

	auto SkyboxPass::setEnvironmentMap(const ImageHandle &p_environment_map) -> void
	{
		m_environmentMap = p_environment_map;
	}

	auto SkyboxPass::_construct(bool32 p_msaa) -> void
	{
		m_skyboxState = m_renderCtx->createUnique<GraphicsState>();
		m_skyboxState->setShaders({m_renderCtx->getGlobals()->getShader("Skybox_VS"), m_renderCtx->getGlobals()->getShader("Skybox_PS")}).setAttachmentCount(1u).
				setCullMode(vk::CullModeFlagBits::eNone).setVertexBufferLayout(RenderContext::fullscreenQuadVbl).setEnableDepthTest(false).setEnableDepthWrite(false).
				setEnableMultisample(p_msaa);
	}
}
