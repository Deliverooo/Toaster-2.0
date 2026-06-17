#include "toast_render/dynamic_renderer_2d.hpp"

#include "toast_render/globals.hpp"

namespace toaster::render
{
	DynamicRenderer2D::DynamicRenderer2D(RenderContext &p_render_ctx, tsm::uint2 p_viewport_size) : m_renderCtx(&p_render_ctx)
	{
		m_specInfo.renderTargetSize = p_viewport_size;
		m_maxVertices               = m_specInfo.maxQuads * 4u;
		m_maxIndices                = m_specInfo.maxQuads * 6u;

		_construct();
	}

	DynamicRenderer2D::DynamicRenderer2D(RenderContext &p_render_ctx, const DynamicRenderer2DSpecInfo &p_create_info) : m_renderCtx(&p_render_ctx),
																														m_specInfo(p_create_info),
																														m_maxVertices(p_create_info.maxQuads * 4u),
																														m_maxIndices(p_create_info.maxQuads * 6u)
	{
		_construct();
	}

	DynamicRenderer2D::~DynamicRenderer2D()
	{
		delete[] m_quadVertexBase;
	}

	auto DynamicRenderer2D::submitQuad(Dx::FXMMATRIX p_transform, const tsm::float4 &p_colour) -> void
	{
		if (m_quadIndexCount >= m_maxIndices)
			TST_PERMA_ASSERT(false);

		for (uint32 i{0u}; i < 4u; ++i)
		{
			// m_quadVertexPtr->position = m_quadVertexPositions[i];
			Dx::XMStoreFloat4(&m_quadVertexPtr->position, Dx::XMVector4Transform(Dx::XMLoadFloat4(&m_quadVertexPositions[i]), p_transform));
			m_quadVertexPtr->colour   = p_colour;
			m_quadVertexPtr->texCoord = m_quadVertexTexCoords[i];
			m_quadVertexPtr->texIndex = static_cast<float32>(m_renderCtx->getGlobals()->debugImage()->getAlignedShaderReadHeapID());
			m_quadVertexPtr++;
		}
		m_quadIndexCount += 6u;
	}

	auto DynamicRenderer2D::submitQuad(Dx::FXMVECTOR p_position, Dx::FXMVECTOR p_scale, const tsm::float4 &p_colour) -> void
	{
		Dx::XMMATRIX transform{Dx::XMMatrixTransformation2D(Dx::XMVectorZero(), 0.0f, p_scale, Dx::XMVectorZero(), 0.0f, p_position)};
		submitQuad(transform, p_colour);
	}

	auto DynamicRenderer2D::submitQuad(Dx::FXMMATRIX p_transform, const ImageHandle &p_image, float32 p_tiling_factor, const tsm::float4 &p_tint_colour) -> void
	{
		if (m_quadIndexCount >= m_maxIndices)
			TST_PERMA_ASSERT(false);

		for (uint32 i{0u}; i < 4u; ++i)
		{
			Dx::XMStoreFloat4(&m_quadVertexPtr->position, Dx::XMVector4Transform(Dx::XMLoadFloat4(&m_quadVertexPositions[i]), p_transform));
			m_quadVertexPtr->colour       = p_tint_colour;
			m_quadVertexPtr->texCoord     = m_quadVertexTexCoords[i];
			m_quadVertexPtr->texIndex     = static_cast<float32>(p_image->getAlignedShaderReadHeapID());
			m_quadVertexPtr->tilingFactor = p_tiling_factor;
			m_quadVertexPtr++;
		}
		m_quadIndexCount += 6u;
	}

	auto DynamicRenderer2D::submitQuad(Dx::FXMVECTOR      p_position, Dx::FXMVECTOR p_scale, const ImageHandle &p_image, float32 p_tiling_factor,
									   const tsm::float4 &p_tint_colour) -> void
	{
		Dx::XMMATRIX transform{Dx::XMMatrixTransformation2D(Dx::XMVectorZero(), 0.0f, p_scale, Dx::XMVectorZero(), 0.0f, p_position)};
		submitQuad(transform, p_image, p_tiling_factor, p_tint_colour);
	}

	auto DynamicRenderer2D::render(Dx::FXMMATRIX            p_view, Dx::CXMMATRIX p_projection, RenderingAttachmentInfo *p_override_colour_attachment,
								   RenderingAttachmentInfo *p_override_depth_attachment) -> void
	{
		// Update the camera uniform buffers
		Globals::ViewProjCameraUB camera_ub{};
		Dx::XMStoreFloat4x4(&camera_ub.viewMatrix, p_view);
		Dx::XMStoreFloat4x4(&camera_ub.projectionMatrix, p_projection);
		m_cameraUBOs->setData(camera_ub);

		#pragma region rendering info
		RenderingInfo rendering_info{};
		rendering_info.renderArea = getRenderingArea(m_specInfo.renderTargetSize);

		auto &colour_attachment{rendering_info.colourAttachments.emplace_back()};
		if (p_override_colour_attachment)
			colour_attachment = *p_override_colour_attachment;
		else
		{
			if (m_specInfo.msaa)
				colour_attachment = getRenderingAttachmentInfo(*m_MSAAColourImage, *m_colourImage->getImage());
			else
				colour_attachment = getRenderingAttachmentInfo(*m_colourImage->getImage());

			colour_attachment.clearValue = vk::ClearColorValue{1.0f, 1.0f, 1.0f, 1.0f};
		}

		if (p_override_depth_attachment)
			rendering_info.depthAttachment = *p_override_depth_attachment;
		else
		{
			if (m_specInfo.msaa)
				rendering_info.depthAttachment = getRenderingAttachmentInfo(*m_MSAADepthImage, *m_depthImage->getImage());
			else
				rendering_info.depthAttachment = getRenderingAttachmentInfo(*m_depthImage->getImage());
		}

		#pragma endregion

		m_graphicsState->bind();
		m_renderCtx->beginRendering(rendering_info);

		const auto size = static_cast<uint32>(reinterpret_cast<uint8 *>(m_quadVertexPtr) - reinterpret_cast<uint8 *>(m_quadVertexBase));
		if (size)
		{
			auto cmd{m_renderCtx->getCurrentSwapchainCommandBuffer()};
			m_quadVertexBuffer->setData(m_quadVertexBase, size, 0);

			QuadConstants quad_constants{};
			quad_constants.cameraAddress = m_cameraUBOs->getDeviceAddress();
			quad_constants.samplerOffset = m_renderCtx->getSampler(ESamplerType::eNearest);
			quad_constants.textureOffset = m_renderCtx->getGlobals()->debugImage()->getAlignedShaderReadHeapID();
			cmd->pushData(quad_constants);

			m_quadVertexBuffer->bind();
			m_quadIndexBuffer->bind();

			cmd->drawIndexed(m_quadIndexCount);
		}

		m_renderCtx->endRendering(rendering_info);

		m_quadIndexCount = 0u;
		m_quadVertexPtr  = m_quadVertexBase;
	}

	auto DynamicRenderer2D::onResize(tsm::uint2 p_size) -> void
	{
		m_specInfo.renderTargetSize = p_size;

		if (!m_specInfo.overrideAttachments)
		{
			if (m_specInfo.msaa)
			{
				m_MSAAColourImage->resize(m_specInfo.renderTargetSize);
				m_MSAADepthImage->resize(m_specInfo.renderTargetSize);
			}

			m_colourImage->resize(m_specInfo.renderTargetSize);
			m_depthImage->resize(m_specInfo.renderTargetSize);
		}
	}

	auto DynamicRenderer2D::_construct() -> void
	{
		// If we are not overriding the attachments, create them.
		if (!m_specInfo.overrideAttachments)
		{
			if (m_specInfo.msaa)
			{
				m_MSAAColourImage = m_renderCtx->createMultisampleAttachmentImage(m_specInfo.renderTargetSize, vk::ImageAspectFlagBits::eColor);
				m_MSAADepthImage  = m_renderCtx->createMultisampleAttachmentImage(m_specInfo.renderTargetSize, vk::ImageAspectFlagBits::eDepth);
			}

			m_colourImage = m_renderCtx->createAttachmentImage(m_specInfo.renderTargetSize, vk::ImageAspectFlagBits::eColor);
			m_depthImage  = m_renderCtx->createAttachmentImage(m_specInfo.renderTargetSize, vk::ImageAspectFlagBits::eDepth);
		}

		m_cameraUBOs = m_renderCtx->createUnique<UniformBufferPFF>(sizeof(Globals::ViewProjCameraUB));

		m_graphicsState = m_renderCtx->createUnique<GraphicsState>();
		auto quad_vs{m_renderCtx->getGlobals()->dynamicShaderLibrary().get("Quad_VS")};
		auto quad_ps{m_renderCtx->getGlobals()->dynamicShaderLibrary().get("Quad_PS")};
		m_graphicsState->setShaders({quad_vs, quad_ps}).setVertexBufferLayout(quadVbl).setAttachmentCount(1u).setCullMode(vk::CullModeFlagBits::eNone).
				setEnableMultisample(m_specInfo.msaa).setEnableDepthTest(true).setEnableDepthWrite(true);

		m_quadVertexBase = new QuadVertex[m_maxVertices];
		m_quadIndexCount = 0u;
		m_quadVertexPtr  = m_quadVertexBase;

		std::vector<uint32> quad_indices(m_maxIndices);
		uint32              offset{0u};
		for (uint32 i{0u}; i < m_maxIndices; i += 6u)
		{
			quad_indices[i + 0] = offset + 0;
			quad_indices[i + 1] = offset + 1;
			quad_indices[i + 2] = offset + 3;

			quad_indices[i + 3] = offset + 1;
			quad_indices[i + 4] = offset + 2;
			quad_indices[i + 5] = offset + 3;

			offset += 4u;
		}

		m_quadVertexBuffer = m_renderCtx->createGPUUnique<gpu::VertexBuffer>(m_maxVertices * sizeof(QuadVertex));
		m_quadIndexBuffer  = m_renderCtx->createGPUUnique<gpu::IndexBuffer>(quad_indices.data(), m_maxIndices * sizeof(uint32));

		m_quadVertexPositions[0] = {0.5f, 0.5f, 0.0f, 1.0f};
		m_quadVertexPositions[1] = {0.5f, -0.5f, 0.0f, 1.0f};
		m_quadVertexPositions[2] = {-0.5f, -0.5f, 0.0f, 1.0f};
		m_quadVertexPositions[3] = {-0.5f, 0.5f, 0.0f, 1.0f};

		m_quadVertexTexCoords[0] = {1.0f, 0.0f};
		m_quadVertexTexCoords[1] = {1.0f, 1.0f};
		m_quadVertexTexCoords[2] = {0.0f, 1.0f};
		m_quadVertexTexCoords[3] = {0.0f, 0.0f};

		m_textureSlots[0] = m_renderCtx->getGlobals()->whiteImage();
	}
}
