#include "toast_render/renderer_2d.hpp"

#include "toast_render/render_context.hpp"

#include "toast_render/globals.hpp"

namespace toaster::render
{
	Renderer2D::Renderer2D(RenderContext *p_render_ctx, const Renderer2DSpecInfo &p_create_info) : m_renderCtx(p_render_ctx), m_specInfo(p_create_info),
																								   m_maxVertices(p_create_info.maxQuads * 4u),
																								   m_maxIndices(p_create_info.maxQuads * 6u)
	{
		m_quadVertexBufferLayout = gpu::BufferLayout{
			{gpu::EBufferDataType::eFloat4, "a_Position"},
			{gpu::EBufferDataType::eFloat4, "a_Colour"},
			{gpu::EBufferDataType::eFloat2, "a_TexCoord"},
			{gpu::EBufferDataType::eFloat, "a_TexIndex"},
			{gpu::EBufferDataType::eFloat, "a_TilingFactor"},
		};

		auto                  quad_shader{m_renderCtx->getGlobals()->shaderLibrary().get("Quad")};
		gpu::PipelineSpecInfo pipeline_create_info{};
		pipeline_create_info.colourAttachments  = {vk::Format::eR8G8B8A8Srgb};
		pipeline_create_info.depthFormat        = m_renderCtx->getPhysicalDevice()->getDepthFormat();
		pipeline_create_info.vertexBufferLayout = m_quadVertexBufferLayout;
		pipeline_create_info.shader             = quad_shader;
		pipeline_create_info.multisample        = m_specInfo.msaa;
		pipeline_create_info.cullMode           = vk::CullModeFlagBits::eNone;
		m_quadPipeline                          = m_renderCtx->createGPU<gpu::VKPipeline>(pipeline_create_info);

		m_cameraUBs       = m_renderCtx->createUniformBuffers<CameraUB>(RenderContext::maxFramesInFlight);
		m_mappedCameraUBs = m_cameraUBs->mapAllMemory(sizeof(CameraUB));

		m_quadRenderPass = m_renderCtx->createGPU<gpu::VKRenderPass>(m_quadPipeline);
		m_quadRenderPass->setInput("Camera", m_cameraUBs);
		m_quadRenderPass->bake();

		m_quadMaterial = make_reference<Material>(m_renderCtx, quad_shader);

		if (!m_specInfo.overrideAttachments)
		{
			m_renderTargetTexture    = m_renderCtx->createAttachmentTexture(m_specInfo.renderTargetSize, vk::ImageAspectFlagBits::eColor);
			m_renderTargetDepthImage = m_renderCtx->createAttachmentImage(m_specInfo.renderTargetSize, vk::ImageAspectFlagBits::eDepth);
		}
		else
		{
			m_renderTargetTexture    = nullptr;
			m_renderTargetDepthImage = nullptr;
		}

		vk::DeviceSize quad_vertex_buffer_size{sizeof(QuadVertex) * m_maxVertices};
		m_quadVertexBuffer = m_renderCtx->createGPU<gpu::VKVertexBuffer>(quad_vertex_buffer_size);
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
		m_quadIndexBuffer = m_renderCtx->createGPU<gpu::VKIndexBuffer>(quad_indices, index_buffer_size);

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

	Renderer2D::~Renderer2D()
	{
		m_cameraUBs->unmapAllMemory();
		delete[] m_quadVertexBase;
	}

	auto Renderer2D::begin(const tsm::float4x4 &p_view_matrix, const tsm::float4x4 &p_proj_matrix) -> void
	{
		CameraUB ubo{};
		ubo.view       = p_view_matrix;
		ubo.proj       = p_proj_matrix;
		ubo.proj[1][1] *= -1.0f;

		std::memcpy(m_mappedCameraUBs[m_renderCtx->getCurrentFrameIndex()], &ubo, sizeof(CameraUB));

		m_quadIndexCount = 0u;
		m_quadVertexPtr  = m_quadVertexBase;

		m_textureSlotIndex = 1u;
		for (uint32 i{1u}; i < m_textureSlots.size(); ++i)
			m_textureSlots[i] = nullptr;

		m_stats.quadCount = 0u;
	}

	auto Renderer2D::end(gpu::VKCommandBuffer *              p_cmd, gpu::RenderingAttachmentInfo *p_override_colour_attachment,
						 const gpu::RenderingAttachmentInfo *p_override_depth_attachment) -> void
	{
		if (m_specInfo.overrideAttachments && !p_override_colour_attachment && !p_override_depth_attachment)
		{
			TST_ASSERT_MSG(false, "Please provide the attachment infos...");
		}

		gpu::RenderingInfo rendering_info{};
		rendering_info.renderArea = vk::Rect2D{{0, 0}, {m_specInfo.renderTargetSize.x, m_specInfo.renderTargetSize.y}};
		rendering_info.layerCount = 1;

		if (p_override_colour_attachment)
			rendering_info.colourAttachments.emplace_back(*p_override_colour_attachment);
		else
		{
			gpu::RenderingAttachmentInfo &colour_attachment_info{rendering_info.colourAttachments.emplace_back()};
			colour_attachment_info.image = m_renderTargetTexture->getImage();
		}

		gpu::RenderingAttachmentInfo depth_attachment_info{};
		if (p_override_depth_attachment)
			depth_attachment_info = *p_override_depth_attachment;
		else
		{
			depth_attachment_info.image      = m_renderTargetDepthImage;
			depth_attachment_info.clearValue = vk::ClearDepthStencilValue{1.0f, 0u};
		}
		rendering_info.depthAttachment = depth_attachment_info;

		m_renderCtx->beginRendering(p_cmd, rendering_info, m_quadRenderPass);

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

			m_renderCtx->renderGeometry(p_cmd, m_quadPipeline, m_quadVertexBuffer, m_quadIndexBuffer, m_quadIndexCount, m_quadMaterial, tsm::float4x4{1.0f});
		}

		m_renderCtx->endRendering(p_cmd, rendering_info);
	}

	auto Renderer2D::submitQuad(const tsm::float3 &p_position, const tsm::float2 &p_scale, const tsm::float4 &p_colour) -> void
	{
		const tsm::float4x4 transform{tsm::translate(tsm::float4x4{1.0f}, p_position) * tsm::scale(tsm::float4x4{1.0f}, {p_scale.x, p_scale.y, 1.0f})};
		submitQuad(transform, p_colour);
	}

	auto Renderer2D::submitQuad(const tsm::float2 &p_position, const tsm::float2 &p_scale, const tsm::float4 &p_colour) -> void
	{
		const tsm::float4x4 transform{
			tsm::translate(tsm::float4x4{1.0f}, tsm::float3{p_position.x, p_position.y, 0.0f}) * tsm::scale(tsm::float4x4{1.0f}, {p_scale.x, p_scale.y, 1.0f})
		};
		submitQuad(transform, p_colour);
	}

	auto Renderer2D::submitQuad(const tsm::float4x4 &p_transform, const tsm::float4 &p_colour) -> void
	{
		if (m_quadIndexCount >= m_maxIndices)
			_beginNewBatch();

		for (uint32 i{0u}; i < 4u; ++i)
		{
			m_quadVertexPtr->position = p_transform * m_quadVertexPositions[i];
			m_quadVertexPtr->colour   = p_colour;
			m_quadVertexPtr->texCoord = m_quadVertexTexCoords[i];
			m_quadVertexPtr->texIndex = 0;
			m_quadVertexPtr++;
		}
		m_quadIndexCount += 6u;
		m_stats.quadCount++;
	}

	auto Renderer2D::submitQuad(const tsm::float4x4 &p_transform, const gpu::Texture2DHandle &p_texture, const tsm::float4 &p_colour) -> void
	{
		if (m_quadIndexCount >= m_maxIndices)
			_beginNewBatch();

		uint32 tex_index{_getTextureSlotIndex(p_texture)};

		for (uint32 i{0u}; i < 4u; ++i)
		{
			m_quadVertexPtr->position = p_transform * m_quadVertexPositions[i];
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

	auto Renderer2D::_beginNewBatch() -> void
	{
		// end();

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
}
