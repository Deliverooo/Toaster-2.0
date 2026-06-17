#include "toast_render/renderer_2d.hpp"

#include "toast_render/render_context.hpp"

#include "toast_render/globals.hpp"

namespace toaster::render
{
	Renderer2D::Renderer2D(RenderContext &p_render_ctx, tsm::uint2 p_viewport_size) : m_renderCtx(&p_render_ctx), m_specInfo({})
	{
		m_specInfo.renderTargetSize = p_viewport_size;
		m_maxVertices               = m_specInfo.maxQuads * 4u;
		m_maxIndices                = m_specInfo.maxQuads * 6u;
		_construct();
	}

	Renderer2D::Renderer2D(RenderContext &p_render_ctx, const Renderer2DSpecInfo &p_create_info) : m_renderCtx(&p_render_ctx), m_specInfo(p_create_info),
																								   m_maxVertices(p_create_info.maxQuads * 4u),
																								   m_maxIndices(p_create_info.maxQuads * 6u)
	{
		_construct();
	}

	Renderer2D::~Renderer2D()
	{
		m_cameraUBs->unmapAllMemory();
		delete[] m_quadVertexBase;
	}

	auto Renderer2D::begin(Dx::FXMMATRIX            p_view, Dx::CXMMATRIX p_projection, RenderingAttachmentInfo *p_override_colour_attachment,
						   RenderingAttachmentInfo *p_override_depth_attachment) -> void
	{
		if (m_specInfo.overrideAttachments && !p_override_colour_attachment && !p_override_depth_attachment)
		{
			TST_ASSERT_MSG(false, "Please provide the attachment infos...");
		}
		m_colourAttachmentInfo = p_override_colour_attachment;
		m_depthAttachmentInfo  = p_override_depth_attachment;

		CameraUB ubo{};
		Dx::XMStoreFloat4x4(&ubo.view, p_view);
		Dx::XMStoreFloat4x4(&ubo.proj, p_projection);

		std::memcpy(m_mappedCameraUBs[m_renderCtx->getCurrentFrameIndex()], &ubo, sizeof(CameraUB));

		m_quadIndexCount = 0u;
		m_quadVertexPtr  = m_quadVertexBase;

		m_textureSlotIndex = 1u;
		for (uint32 i{1u}; i < m_textureSlots.size(); ++i)
			m_textureSlots[i] = nullptr;

		m_stats.quadCount = 0u;
	}

	auto Renderer2D::end(gpu::VKCommandBuffer *p_cmd) -> void
	{
		if (!p_cmd)
			p_cmd = m_renderCtx->getCurrentSwapchainCommandBuffer();

		RenderingInfo rendering_info{};
		rendering_info.renderArea = vk::Rect2D{{0, 0}, {m_specInfo.renderTargetSize.x, m_specInfo.renderTargetSize.y}};
		rendering_info.layerCount = 1;

		if (m_colourAttachmentInfo)
			rendering_info.colourAttachments.emplace_back(*m_colourAttachmentInfo);
		else
		{
			RenderingAttachmentInfo &colour_attachment_info{rendering_info.colourAttachments.emplace_back()};
			colour_attachment_info.image = m_renderTargetTexture->getImage();
		}

		RenderingAttachmentInfo depth_attachment_info{};
		if (m_depthAttachmentInfo)
			depth_attachment_info = *m_depthAttachmentInfo;
		else
		{
			depth_attachment_info.image      = m_renderTargetDepthImage;
			depth_attachment_info.clearValue = vk::ClearDepthStencilValue{1.0f, 0u};
		}

		rendering_info.depthAttachment = depth_attachment_info;

		m_renderCtx->beginRenderPass(rendering_info, m_quadRenderPass, p_cmd);

		const auto size = static_cast<uint32>(reinterpret_cast<uint8 *>(m_quadVertexPtr) - reinterpret_cast<uint8 *>(m_quadVertexBase));
		if (size) // Apparently you have to check ts, or things won't work correctly and there will be artifacts...
		{
			m_quadVertexBuffer->setData(m_quadVertexBase, size, 0);

			for (uint32 i{0u}; i < m_textureSlots.size(); ++i)
			{
				if (m_textureSlots[i])
					m_quadMaterial->setTexture("u_Textures", m_textureSlots[i], i);
				else
					m_quadMaterial->setTexture("u_Textures", m_renderCtx->getGlobals()->whiteTexture(), i);
			}

			m_renderCtx->renderGeometry(m_quadPipeline, m_quadVertexBuffer, m_quadIndexBuffer, m_quadIndexCount, m_quadMaterial, Dx::XMMatrixIdentity(), p_cmd);
		}

		m_renderCtx->endRenderPass(rendering_info, p_cmd);
	}

	auto Renderer2D::submitQuad(Dx::FXMVECTOR p_position, Dx::FXMVECTOR p_scale, const tsm::float4 &p_colour) -> void
	{
		Dx::XMMATRIX transform{Dx::XMMatrixTransformation2D(Dx::XMVectorZero(), 0.0f, p_scale, Dx::XMVectorZero(), 0.0f, p_position)};
		submitQuad(transform, p_colour);
	}

	auto Renderer2D::submitQuad(Dx::FXMMATRIX p_transform, const tsm::float4 &p_colour) -> void
	{
		if (m_quadIndexCount >= m_maxIndices)
			_beginNewBatch();

		for (uint32 i{0u}; i < 4u; ++i)
		{
			Dx::XMStoreFloat4(&m_quadVertexPtr->position, Dx::XMVector4Transform(Dx::XMLoadFloat4(&m_quadVertexPositions[i]), p_transform));
			m_quadVertexPtr->colour   = p_colour;
			m_quadVertexPtr->texCoord = m_quadVertexTexCoords[i];
			m_quadVertexPtr->texIndex = 0;
			m_quadVertexPtr++;
		}
		m_quadIndexCount += 6u;
		m_stats.quadCount++;
	}

	auto Renderer2D::submitQuad(Dx::FXMMATRIX p_transform, const gpu::Texture2DHandle &p_texture, const tsm::float4 &p_colour) -> void
	{
		if (m_quadIndexCount >= m_maxIndices)
			_beginNewBatch();

		uint32 tex_index{_getTextureSlotIndex(p_texture)};

		for (uint32 i{0u}; i < 4u; ++i)
		{
			Dx::XMStoreFloat4(&m_quadVertexPtr->position, Dx::XMVector4Transform(Dx::XMLoadFloat4(&m_quadVertexPositions[i]), p_transform));
			m_quadVertexPtr->colour   = p_colour;
			m_quadVertexPtr->texCoord = m_quadVertexTexCoords[i];
			m_quadVertexPtr->texIndex = static_cast<float32>(tex_index);
			m_quadVertexPtr++;
		}
		m_quadIndexCount += 6u;
		m_stats.quadCount++;
	}

	auto Renderer2D::getStats() const -> const Stats &
	{
		return m_stats;
	}

	auto Renderer2D::getOutputColourTexture() const -> const gpu::Texture2DHandle &
	{
		return m_renderTargetTexture;
	}

	auto Renderer2D::onResize(tsm::uint2 p_size) -> void
	{
		m_specInfo.renderTargetSize = p_size;

		if (!m_specInfo.overrideAttachments)
		{
			m_renderTargetTexture->resize(p_size);
			m_renderTargetDepthImage->resize(p_size);
		}
	}

	auto Renderer2D::_construct() -> void
	{
		// auto                  quad_shader{m_renderCtx->getGlobals()->shaderLibrary().get("Quad")};
		gpu::PipelineSpecInfo pipeline_create_info{};
		pipeline_create_info.colourAttachments  = {vk::Format::eR8G8B8A8Srgb};
		pipeline_create_info.depthFormat        = m_renderCtx->getPhysicalDevice()->getDepthFormat();
		pipeline_create_info.vertexBufferLayout = quadVbl;
		// pipeline_create_info.shader             = quad_shader;
		pipeline_create_info.multisample        = m_specInfo.msaa;
		pipeline_create_info.cullMode           = vk::CullModeFlagBits::eNone;
		m_quadPipeline                          = m_renderCtx->createGPURef<gpu::VKPipeline>(pipeline_create_info, "Quad");

		m_cameraUBs       = m_renderCtx->createUniformBuffers<CameraUB>(RenderContext::maxFramesInFlight);
		m_mappedCameraUBs = m_cameraUBs->mapAllMemory(sizeof(CameraUB));

		m_quadRenderPass = m_renderCtx->createRef<RenderPass>(m_quadPipeline);
		m_quadRenderPass->setInput("Camera", m_cameraUBs).bake();

		// m_quadMaterial = m_renderCtx->createRef<Material>(quad_shader);

		if (!m_specInfo.overrideAttachments)
		{
			m_renderTargetTexture    = m_renderCtx->createAttachmentTexture(m_specInfo.renderTargetSize, vk::ImageAspectFlagBits::eColor);
			m_renderTargetDepthImage = m_renderCtx->createAttachmentImageRaw(m_specInfo.renderTargetSize, vk::ImageAspectFlagBits::eDepth);
		}
		else
		{
			m_renderTargetTexture    = nullptr;
			m_renderTargetDepthImage = nullptr;
		}

		vk::DeviceSize quad_vertex_buffer_size{sizeof(QuadVertex) * m_maxVertices};
		m_quadVertexBuffer = m_renderCtx->createGPURef<gpu::VKVertexBuffer>(quad_vertex_buffer_size);
		m_quadVertexBase   = new QuadVertex[m_maxVertices];

		auto   quad_indices{new uint32[m_maxIndices]};
		uint32 offset{0u};
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

		vk::DeviceSize index_buffer_size{m_maxIndices * sizeof(uint32)};
		m_quadIndexBuffer = m_renderCtx->createGPURef<gpu::VKIndexBuffer>(quad_indices, index_buffer_size);

		delete[] quad_indices;

		m_quadVertexPositions[0] = {0.5f, 0.5f, 0.0f, 1.0f};
		m_quadVertexPositions[1] = {0.5f, -0.5f, 0.0f, 1.0f};
		m_quadVertexPositions[2] = {-0.5f, -0.5f, 0.0f, 1.0f};
		m_quadVertexPositions[3] = {-0.5f, 0.5f, 0.0f, 1.0f};

		m_quadVertexTexCoords[0] = {1.0f, 0.0f};
		m_quadVertexTexCoords[1] = {1.0f, 1.0f};
		m_quadVertexTexCoords[2] = {0.0f, 1.0f};
		m_quadVertexTexCoords[3] = {0.0f, 0.0f};

		m_textureSlots[0] = m_renderCtx->getGlobals()->whiteTexture();
	}

	auto Renderer2D::_beginNewBatch() -> void
	{
		end();

		m_quadIndexCount = 0u;
		m_quadVertexPtr  = m_quadVertexBase;
	}

	auto Renderer2D::_getTextureSlotIndex(const gpu::Texture2DHandle &p_texture) -> uint32
	{
		uint32 texture_index{0u};
		for (uint32 i{1u}; i < m_textureSlotIndex; ++i)
		{
			if (m_textureSlots[i]->getDescriptorInfo() == p_texture->getDescriptorInfo())
			{
				texture_index = i;
				break;
			}
		}

		if (texture_index == 0u)
		{
			// LOG_TRACE("Setting new texture: {}", p_texture->getPath().string());
			texture_index                      = m_textureSlotIndex;
			m_textureSlots[m_textureSlotIndex] = p_texture;
			++m_textureSlotIndex;
		}

		return texture_index;
	}
	#if 1

	Renderer2DV2::Renderer2DV2(RenderContext &p_render_ctx, tsm::uint2 p_viewport_size) : m_renderCtx(&p_render_ctx), m_specInfo({})
	{
		m_specInfo.renderTargetSize = p_viewport_size;
		m_maxVertices               = m_specInfo.maxQuads * 4u;
		m_maxIndices                = m_specInfo.maxQuads * 6u;
		_construct();
	}

	Renderer2DV2::Renderer2DV2(RenderContext &p_render_ctx, const Renderer2DSpecInfo &p_create_info) : m_renderCtx(&p_render_ctx), m_specInfo(p_create_info),
																									   m_maxVertices(p_create_info.maxQuads * 4u),
																									   m_maxIndices(p_create_info.maxQuads * 6u)
	{
		_construct();
	}

	Renderer2DV2::~Renderer2DV2()
	{
		// m_cameraUBs->unmapAllMemory();
		delete[] m_quadVertexBase;
	}

	auto Renderer2DV2::begin(Dx::FXMMATRIX            p_view, Dx::CXMMATRIX p_projection, RenderingAttachmentInfo *p_override_colour_attachment,
							 RenderingAttachmentInfo *p_override_depth_attachment) -> void
	{
		if (m_specInfo.overrideAttachments && !p_override_colour_attachment && !p_override_depth_attachment)
		{
			TST_PERMA_ASSERT_MSG(false, "Please provide the attachment infos...");
		}
		m_colourAttachmentInfo = p_override_colour_attachment;
		m_depthAttachmentInfo  = p_override_depth_attachment;

		CameraUB ubo{};
		Dx::XMStoreFloat4x4(&ubo.view, p_view);
		Dx::XMStoreFloat4x4(&ubo.proj, p_projection);
		m_cameraUBs->setData(ubo);

		m_quadIndexCount = 0u;
		m_quadVertexPtr  = m_quadVertexBase;

		m_textureSlotIndex = 1u;
		// for (uint32 i{1u}; i < m_textureSlots.size(); ++i)
		// m_textureSlots[i] = nullptr;
	}

	auto Renderer2DV2::end(gpu::VKCommandBuffer *p_cmd) -> void
	{
		if (!p_cmd)
			p_cmd = m_renderCtx->getCurrentSwapchainCommandBuffer();

		RenderingInfo rendering_info{};
		rendering_info.renderArea = vk::Rect2D{{0, 0}, {m_specInfo.renderTargetSize.x, m_specInfo.renderTargetSize.y}};
		rendering_info.layerCount = 1;

		if (m_colourAttachmentInfo)
			rendering_info.colourAttachments.emplace_back(*m_colourAttachmentInfo);
		else
		{
			RenderingAttachmentInfo &colour_attachment_info{rendering_info.colourAttachments.emplace_back()};
			colour_attachment_info.image = m_renderTargetImage->getImage();
		}

		RenderingAttachmentInfo depth_attachment_info{};
		if (m_depthAttachmentInfo)
			depth_attachment_info = *m_depthAttachmentInfo;
		else
		{
			depth_attachment_info.image      = m_renderTargetDepthImage;
			depth_attachment_info.clearValue = vk::ClearDepthStencilValue{1.0f, 0u};
		}

		rendering_info.depthAttachment = depth_attachment_info;

		m_renderCtx->beginRendering(rendering_info, p_cmd);
		m_graphicsState->bind();

		const auto size = static_cast<uint32>(reinterpret_cast<uint8 *>(m_quadVertexPtr) - reinterpret_cast<uint8 *>(m_quadVertexBase));
		if (size) // Apparently you have to check ts, or things won't work correctly and there will be artifacts...
		{
			m_quadVertexBuffer->setData(m_quadVertexBase, size, 0);

			TST_PUSH_CONSTANT_BLOCK(PushConstants)
			{
				uint32 textureIndex;
				uint32 samplerIndex;

				uintptr currentCameraUBOPtr;
			};
			PushConstants pcs{};
			pcs.currentCameraUBOPtr = m_cameraUBs->getDeviceAddress();

			for (uint32 i{0u}; i < m_textureSlots.size(); ++i)
			{
				if (m_textureSlots[i])
					pcs.textureIndex = m_textureSlots[i]->getAlignedShaderReadHeapID();
				else
					pcs.textureIndex = m_renderCtx->getGlobals()->whiteImage()->getAlignedShaderReadHeapID();
			}

			p_cmd->pushData(pcs);

			m_quadVertexBuffer->bind();
			m_quadIndexBuffer->bind();

			p_cmd->drawIndexed(m_quadIndexCount);
		}

		m_renderCtx->endRendering(rendering_info);
	}

	auto Renderer2DV2::submitQuad(Dx::FXMVECTOR p_position, Dx::FXMVECTOR p_scale, const tsm::float4 &p_colour) -> void
	{
		Dx::XMMATRIX transform{Dx::XMMatrixTransformation2D(Dx::XMVectorZero(), 0.0f, p_scale, Dx::XMVectorZero(), 0.0f, p_position)};
		submitQuad(transform, p_colour);
	}

	auto Renderer2DV2::submitQuad(Dx::FXMMATRIX p_transform, const tsm::float4 &p_colour) -> void
	{
		if (m_quadIndexCount >= m_maxIndices)
			_beginNewBatch();

		for (uint32 i{0u}; i < 4u; ++i)
		{
			Dx::XMStoreFloat4(&m_quadVertexPtr->position, Dx::XMVector4Transform(Dx::XMLoadFloat4(&m_quadVertexPositions[i]), p_transform));
			m_quadVertexPtr->colour   = p_colour;
			m_quadVertexPtr->texCoord = m_quadVertexTexCoords[i];
			m_quadVertexPtr->texIndex = static_cast<float32>(m_renderCtx->getGlobals()->whiteImage()->getAlignedShaderReadHeapID());
			m_quadVertexPtr++;
		}
		m_quadIndexCount += 6u;
	}

	auto Renderer2DV2::submitQuad(Dx::FXMMATRIX p_transform, const ImageHandle &p_texture, const tsm::float4 &p_colour) -> void
	{
		if (m_quadIndexCount >= m_maxIndices)
			_beginNewBatch();

		uint32 tex_index{_getTextureSlotIndex(p_texture)};

		for (uint32 i{0u}; i < 4u; ++i)
		{
			Dx::XMStoreFloat4(&m_quadVertexPtr->position, Dx::XMVector4Transform(Dx::XMLoadFloat4(&m_quadVertexPositions[i]), p_transform));
			m_quadVertexPtr->colour   = p_colour;
			m_quadVertexPtr->texCoord = m_quadVertexTexCoords[i];
			m_quadVertexPtr->texIndex = static_cast<float32>(p_texture->getAlignedShaderReadHeapID());
			m_quadVertexPtr++;
		}
		m_quadIndexCount += 6u;
	}

	auto Renderer2DV2::getOutputColourImage() const -> const ImageHandle &
	{
		return m_renderTargetImage;
	}

	auto Renderer2DV2::onResize(tsm::uint2 p_size) -> void
	{
		m_specInfo.renderTargetSize = p_size;

		if (!m_specInfo.overrideAttachments)
		{
			m_renderTargetImage->getImage()->resize(p_size);
			m_renderTargetDepthImage->resize(p_size);
		}
	}

	auto Renderer2DV2::_construct() -> void
	{
		m_graphicsState = m_renderCtx->createRef<GraphicsState>();

		auto quad_vs{m_renderCtx->getGlobals()->getShader("Quad_VS")};
		auto quad_ps{m_renderCtx->getGlobals()->getShader("Quad_PS")};

		m_graphicsState->setShaders({quad_vs, quad_ps}).setVertexBufferLayout(quadVbl).setAttachmentCount(1u).setCullMode(vk::CullModeFlagBits::eNone).
				setEnableMultisample(m_specInfo.msaa);

		m_cameraUBs = m_renderCtx->createRef<UniformBufferPFF>(sizeof(CameraUB));

		if (!m_specInfo.overrideAttachments)
		{
			m_renderTargetImage      = m_renderCtx->createAttachmentImage(m_specInfo.renderTargetSize, vk::ImageAspectFlagBits::eColor);
			m_renderTargetDepthImage = m_renderCtx->createAttachmentImageRaw(m_specInfo.renderTargetSize, vk::ImageAspectFlagBits::eDepth);
		}
		else
		{
			m_renderTargetImage      = nullptr;
			m_renderTargetDepthImage = nullptr;
		}

		m_renderTargetImage = m_renderCtx->getGlobals()->whiteImage();

		vk::DeviceSize quad_vertex_buffer_size{sizeof(QuadVertex) * m_maxVertices};
		m_quadVertexBuffer = m_renderCtx->createGPURef<gpu::VKVertexBuffer>(quad_vertex_buffer_size);
		m_quadVertexBase   = new QuadVertex[m_maxVertices];

		auto   quad_indices{new uint32[m_maxIndices]};
		uint32 offset{0u};
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

		vk::DeviceSize index_buffer_size{m_maxIndices * sizeof(uint32)};
		m_quadIndexBuffer = m_renderCtx->createGPURef<gpu::VKIndexBuffer>(quad_indices, index_buffer_size);

		delete[] quad_indices;

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

	auto Renderer2DV2::_beginNewBatch() -> void
	{
		end();

		m_quadIndexCount = 0u;
		m_quadVertexPtr  = m_quadVertexBase;
	}

	auto Renderer2DV2::_getTextureSlotIndex(const ImageHandle &p_texture) -> uint32
	{
		uint32 texture_index{0u};
		for (uint32 i{1u}; i < m_textureSlotIndex; ++i)
		{
			if (m_textureSlots[i]->getShaderReadHeapID() == p_texture->getShaderReadHeapID())
			{
				texture_index = i;
				break;
			}
		}

		if (texture_index == 0u)
		{
			// LOG_TRACE("Setting new texture: {}", p_texture->getPath().string());
			texture_index                      = m_textureSlotIndex;
			m_textureSlots[m_textureSlotIndex] = p_texture;
			++m_textureSlotIndex;
		}

		return texture_index;
	}
	#endif
}
