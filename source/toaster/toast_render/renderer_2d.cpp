#include "renderer_2d.hpp"

#include "globals.hpp"
#include "renderer.hpp"
#include "toast_gpu/vk/vk_gpu_context.hpp"

namespace toaster
{
	Renderer2D::Renderer2D(gpu::VKGPUContext *p_ctx, const Renderer2DCreateInfo &p_create_info) : m_ctx(p_ctx), m_createInfo(p_create_info),
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

		gpu::VKShader::Bytecode    vs_bytecode{io::filesystem::readBinary("shaders/quad.vert.glsl.spv")};
		gpu::VKShader::Bytecode    ps_bytecode{io::filesystem::readBinary("shaders/quad.pixel.glsl.spv")};
		gpu::VKShader::BytecodeMap shader_bytecode_map{{vk::ShaderStageFlagBits::eVertex, vs_bytecode}, {vk::ShaderStageFlagBits::eFragment, ps_bytecode}};
		m_quadShader = m_ctx->alloc<gpu::VKShader>(shader_bytecode_map, "Quad");

		gpu::PipelineCreateInfo pipeline_create_info{};
		pipeline_create_info.colourAttachments  = {vk::Format::eR8G8B8A8Srgb};
		pipeline_create_info.depthFormat        = m_ctx->findDepthFormat();
		pipeline_create_info.vertexBufferLayout = m_quadVertexBufferLayout;
		pipeline_create_info.shader             = m_quadShader;
		pipeline_create_info.cullMode           = vk::CullModeFlagBits::eNone;
		pipeline_create_info.multisample        = false;
		m_quadPipeline                          = m_ctx->alloc<gpu::VKPipeline>(pipeline_create_info);

		constexpr vk::DeviceSize ubo_size{sizeof(CameraUB)};
		m_cameraUBs       = m_ctx->alloc<gpu::VKUniformBufferPFF>(ubo_size, gpu::VKGPUContext::c_maxFramesInFlight);
		m_mappedCameraUBs = m_cameraUBs->mapMemory(ubo_size, 0);

		m_quadRenderPass = m_ctx->alloc<gpu::VKRenderPass>(m_quadPipeline);
		m_quadRenderPass->setInput("Camera", m_cameraUBs);
		m_quadRenderPass->bake();

		m_quadMaterial = m_ctx->alloc<gpu::VKMaterial>(m_quadShader);

		if (!m_createInfo.overrideAttachments)
		{
			gpu::TextureSpecInfo colour_attachment_texture_spec_info{};
			colour_attachment_texture_spec_info.width  = m_createInfo.renderTargetWidth;
			colour_attachment_texture_spec_info.height = m_createInfo.renderTargetHeight;
			colour_attachment_texture_spec_info.format = vk::Format::eR8G8B8A8Srgb;
			m_renderTargetTexture                      = m_ctx->alloc<gpu::VKTexture2D>(colour_attachment_texture_spec_info);

			gpu::ImageCreateInfo depth_attachment_image_create_info{};
			depth_attachment_image_create_info.width  = m_createInfo.renderTargetWidth;
			depth_attachment_image_create_info.height = m_createInfo.renderTargetHeight;
			depth_attachment_image_create_info.format = m_ctx->findDepthFormat();
			depth_attachment_image_create_info.usage  = vk::ImageUsageFlagBits::eDepthStencilAttachment;
			m_renderTargetDepthImage                  = m_ctx->alloc<gpu::VKImage2D>(depth_attachment_image_create_info);
		}
		else
		{
			m_renderTargetTexture    = nullptr;
			m_renderTargetDepthImage = nullptr;
		}

		vk::DeviceSize quad_vertex_buffer_size{sizeof(QuadVertex) * m_maxVertices};
		m_quadVertexBuffer = m_ctx->alloc<gpu::VKVertexBuffer>(quad_vertex_buffer_size);
		m_quadVertexBase   = new QuadVertex[m_maxVertices];

		auto * quad_indices{new uint32[m_maxIndices]};
		uint32 offset{0u};
		for (uint32 i{0u}; i < m_maxIndices; i += 6u)
		{
			quad_indices[i]     = offset;
			quad_indices[i + 1] = offset + 1;
			quad_indices[i + 2] = offset + 2;

			quad_indices[i + 3] = offset + 2;
			quad_indices[i + 4] = offset + 3;
			quad_indices[i + 5] = offset;

			offset += 4u;
		}

		vk::DeviceSize index_buffer_size{m_maxIndices * sizeof(uint32)};
		m_quadIndexBuffer = m_ctx->alloc<gpu::VKIndexBuffer>(quad_indices, index_buffer_size);

		delete[] quad_indices;

		m_quadVertexPositions = {
			tsm::float4{-0.5f, -0.5f, 0.0f, 1.0f},
			tsm::float4{0.5f, -0.5f, 0.0f, 1.0f},
			tsm::float4{0.5f, 0.5f, 0.0f, 1.0f},
			tsm::float4{-0.5f, 0.5f, 0.0f, 1.0f}
		};

		m_quadVertexTexCoords = {tsm::float2{0.0f, 0.0f}, tsm::float2{1.0f, 0.0f}, tsm::float2{1.0f, 1.0f}, tsm::float2{0.0f, 1.0f}};

		m_textureSlots[0] = Globals::getWhiteTexture();
	}

	Renderer2D::~Renderer2D()
	{
		m_cameraUBs->unmapMemory();
		delete[] m_quadVertexBase;
	}

	auto Renderer2D::begin([[maybe_unused]] const vk::raii::CommandBuffer &p_cmd, uint32 p_frame_index, const tsm::float4x4 &p_view_matrix,
						   const tsm::float4x4 &                           p_proj_matrix) -> void
	{
		CameraUB ubo{};
		ubo.view = p_view_matrix;
		ubo.proj = p_proj_matrix;

		ubo.proj[1][1] *= -1.0f;

		std::memcpy(m_mappedCameraUBs[p_frame_index], &ubo, sizeof(CameraUB));

		m_quadIndexCount = 0u;
		m_quadVertexPtr  = m_quadVertexBase;

		m_textureSlotIndex = 1u;
		for (uint32 i{1u}; i < m_textureSlots.size(); ++i)
			m_textureSlots[i] = nullptr;

		m_stats.quadCount = 0u;
	}

	auto Renderer2D::end(const vk::raii::CommandBuffer &p_cmd, uint32 p_frame_index, gpu::RenderingAttachmentInfo *p_override_colour_attachment,
						 gpu::RenderingAttachmentInfo * p_override_depth_attachment) -> void
	{
		if (m_createInfo.overrideAttachments && !p_override_colour_attachment && !p_override_depth_attachment)
			TST_ASSERT_MSG(false, "Please provide the attachment infos...");

		gpu::RenderingInfo rendering_info{};
		rendering_info.renderArea = vk::Rect2D{{0, 0}, {m_createInfo.renderTargetWidth, m_createInfo.renderTargetHeight}};
		rendering_info.layerCount = 1;

		if (p_override_colour_attachment)
			rendering_info.colourAttachments.emplace_back(*p_override_colour_attachment);
		else
		{
			gpu::RenderingAttachmentInfo &colour_attachment_info{rendering_info.colourAttachments.emplace_back()};
			colour_attachment_info.image      = m_renderTargetTexture->getImage();
			colour_attachment_info.clearValue = vk::ClearColorValue{1.0f, 0.0f, 1.0f, 1.0f};
			colour_attachment_info.loadOp     = vk::AttachmentLoadOp::eClear;
			colour_attachment_info.storeOp    = vk::AttachmentStoreOp::eStore;
		}

		if (p_override_depth_attachment)
			rendering_info.pDepthAttachment = p_override_depth_attachment;
		else
		{
			gpu::RenderingAttachmentInfo depth_attachment_info{};
			depth_attachment_info.image      = m_renderTargetDepthImage;
			depth_attachment_info.loadOp     = vk::AttachmentLoadOp::eClear;
			depth_attachment_info.storeOp    = vk::AttachmentStoreOp::eStore;
			depth_attachment_info.clearValue = vk::ClearDepthStencilValue{1.0f, 0u};
			rendering_info.pDepthAttachment  = &depth_attachment_info;
		}

		Renderer::beginRendering(rendering_info, p_cmd, p_frame_index, m_quadRenderPass);

		const auto size = static_cast<uint32>(reinterpret_cast<uint8 *>(m_quadVertexPtr) - reinterpret_cast<uint8 *>(m_quadVertexBase));
		if (size) // Apparently you have to check ts, or things won't work correctly and there will be artifacts...
		{
			m_quadVertexBuffer->setData(m_quadVertexBase, size, 0);

			for (uint32 i{0u}; i < m_textureSlots.size(); ++i)
			{
				if (m_textureSlots[i])
				{
					// LOG_INFO("Index: {} | Texture: {}", i, m_textureSlots[i]->getPath().string());
					m_quadMaterial->set("u_Textures", m_textureSlots[i], i);
				}
				else
					m_quadMaterial->set("u_Textures", Globals::getWhiteTexture(), i);
			}

			Renderer::renderGeometry(p_cmd, p_frame_index, m_quadPipeline, m_quadVertexBuffer, m_quadIndexBuffer, m_quadIndexCount, m_quadMaterial, glm::mat4{1.0f});
		}

		Renderer::endRendering(rendering_info, p_cmd);
	}

	auto Renderer2D::submitQuad(const tsm::float3 &p_position, const tsm::float2 &p_scale, const tsm::float4 &p_colour) -> void
	{
		const tsm::float4x4 transform{glm::translate(glm::mat4{1.0f}, p_position) * glm::scale(glm::mat4{1.0f}, {p_scale.x, p_scale.y, 1.0f})};
		submitQuad(transform, p_colour);
	}

	auto Renderer2D::submitQuad(const tsm::float2 &p_position, const tsm::float2 &p_scale, const tsm::float4 &p_colour) -> void
	{
		const tsm::float4x4 transform{
			glm::translate(glm::mat4{1.0f}, tsm::float3{p_position.x, p_position.y, 0.0f}) * glm::scale(glm::mat4{1.0f}, {p_scale.x, p_scale.y, 1.0f})
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

	auto Renderer2D::submitQuad(const tsm::float4x4 &p_transform, const RefPtr<gpu::VKTexture2D> &p_texture, const tsm::float4 &p_colour) -> void
	{
		if (m_quadIndexCount >= m_maxIndices)
			_beginNewBatch();

		uint32 tex_index{_getTextureSlotIndex(p_texture)};

		for (uint32 i{0u}; i < 4u; ++i)
		{
			m_quadVertexPtr->position = p_transform * m_quadVertexPositions[i];
			m_quadVertexPtr->colour   = p_colour;
			m_quadVertexPtr->texCoord = m_quadVertexTexCoords[i];
			m_quadVertexPtr->texIndex = tex_index;
			m_quadVertexPtr++;
		}
		m_quadIndexCount += 6u;
		m_stats.quadCount++;
	}

	auto Renderer2D::getStats() const -> const Stats &
	{
		return m_stats;
	}

	auto Renderer2D::getColourOutput() const -> const RefPtr<gpu::VKTexture2D> &
	{
		return m_renderTargetTexture;
	}

	auto Renderer2D::onResize(uint32 p_width, uint32 p_height) -> void
	{
		m_createInfo.renderTargetWidth  = p_width;
		m_createInfo.renderTargetHeight = p_height;

		if (!m_createInfo.overrideAttachments)
		{
			m_renderTargetTexture->resize(p_width, p_height);
			m_renderTargetDepthImage->resize(p_width, p_height);
		}
	}

	auto Renderer2D::_beginNewBatch() -> void
	{
		// end();

		m_quadIndexCount = 0u;
		m_quadVertexPtr  = m_quadVertexBase;
	}

	auto Renderer2D::_getTextureSlotIndex(const RefPtr<gpu::VKTexture2D> &p_texture) -> uint32
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
