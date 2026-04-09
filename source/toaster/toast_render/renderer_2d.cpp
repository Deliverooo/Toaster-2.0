#include "renderer_2d.hpp"

#include "globals.hpp"
#include "renderer.hpp"
#include "render_command.hpp"
#include "assimp/Vertex.h"

namespace toaster
{
	Renderer2D::Renderer2D(gpu::VKGPUContext *p_ctx, const Renderer2DCreateInfo &p_create_info) : m_ctx(p_ctx), m_createInfo(p_create_info),
																								  m_maxVertices(p_create_info.maxQuads * 4u),
																								  m_maxIndices(p_create_info.maxQuads * 6u)
	{
		auto &device = m_ctx->getDevice();

		m_quadVertexBufferLayout = gpu::VertexBufferLayout{
			{gpu::EShaderDataType::eFloat4, "a_Position"},
			{gpu::EShaderDataType::eFloat4, "a_Colour"},
			{gpu::EShaderDataType::eFloat2, "a_TexCoord"},
			{gpu::EShaderDataType::eFloat, "a_TexIndex"},
			{gpu::EShaderDataType::eFloat, "a_TilingFactor"},
		};

		gpu::VKShader::Bytecode    vs_bytecode = io::filesystem::readBinary("shaders/quad.vert.glsl.spv");
		gpu::VKShader::Bytecode    ps_bytecode = io::filesystem::readBinary("shaders/quad.pixel.glsl.spv");
		gpu::VKShader::BytecodeMap shader_bytecode_map{{vk::ShaderStageFlagBits::eVertex, vs_bytecode}, {vk::ShaderStageFlagBits::eFragment, ps_bytecode}};
		m_quadShader = make_reference<gpu::VKShader>(m_ctx, shader_bytecode_map);

		gpu::PipelineCreateInfo pipeline_create_info{};
		pipeline_create_info.colourAttachments  = {vk::Format::eR8G8B8A8Srgb};
		pipeline_create_info.depthFormat        = m_ctx->findDepthFormat();
		pipeline_create_info.vertexBufferLayout = m_quadVertexBufferLayout;
		pipeline_create_info.shader             = m_quadShader;
		pipeline_create_info.multisample        = false;
		m_quadPipeline                          = make_reference<gpu::VKPipeline>(m_ctx, pipeline_create_info);

		constexpr vk::DeviceSize ubo_size{sizeof(CameraUB)};
		m_cameraUBs       = make_reference<gpu::VKUniformBufferPFF>(m_ctx, ubo_size, gpu::VKGPUContext::c_maxFramesInFlight);
		m_mappedCameraUBs = m_cameraUBs->mapMemory(ubo_size, 0);

		m_quadRenderPass = make_reference<gpu::VKRenderPass>(m_ctx, m_quadPipeline);
		m_quadRenderPass->setInput("Camera", m_cameraUBs);
		m_quadRenderPass->bake();

		m_quadMaterial = make_reference<gpu::VKMaterial>(m_ctx, m_quadShader);

		gpu::TextureSpecInfo colour_attachment_texture_spec_info{};
		colour_attachment_texture_spec_info.width  = m_createInfo.renderTargetWidth;
		colour_attachment_texture_spec_info.height = m_createInfo.renderTargetHeight;
		colour_attachment_texture_spec_info.format = vk::Format::eR8G8B8A8Srgb;
		m_renderTargetTexture                      = make_reference<gpu::VKTexture2D>(m_ctx, colour_attachment_texture_spec_info);

		gpu::ImageCreateInfo depth_attachment_image_create_info{};
		depth_attachment_image_create_info.width  = m_createInfo.renderTargetWidth;
		depth_attachment_image_create_info.height = m_createInfo.renderTargetHeight;
		depth_attachment_image_create_info.format = m_ctx->findDepthFormat();
		depth_attachment_image_create_info.usage  = vk::ImageUsageFlagBits::eDepthStencilAttachment;
		m_renderTargetDepthImage                  = make_reference<gpu::VKImage2D>(m_ctx, depth_attachment_image_create_info);

		vk::DeviceSize quad_vertex_buffer_size{sizeof(QuadVertex) * m_maxVertices};
		m_quadVertexBuffer = make_reference<gpu::VKVertexBuffer>(m_ctx, quad_vertex_buffer_size);
		m_quadVertexBase   = new QuadVertex[m_maxVertices];

		auto * quad_indices = new uint32[m_maxIndices];
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
		m_quadIndexBuffer = make_reference<gpu::VKIndexBuffer>(m_ctx, quad_indices, index_buffer_size);

		delete[] quad_indices;

		m_quadVertexPositions = {
			tsm::float4{-0.5f, -0.5f, 0.0f, 1.0f},
			tsm::float4{0.5f, -0.5f, 0.0f, 1.0f},
			tsm::float4{0.5f, 0.5f, 0.0f, 1.0f},
			tsm::float4{-0.5f, 0.5f, 0.0f, 1.0f}
		};

		m_quadVertexTexCoords = {tsm::float2{0.0f, 0.0f}, tsm::float2{1.0f, 0.0f}, tsm::float2{1.0f, 1.0f}, tsm::float2{0.0f, 1.0f}};

		uint32               white_image_data{0xFFFFFFFF};
		gpu::TextureSpecInfo white_texture_spec_info{};
		white_texture_spec_info.width        = 1;
		white_texture_spec_info.height       = 1;
		white_texture_spec_info.format       = vk::Format::eR8G8B8A8Unorm;
		white_texture_spec_info.generateMips = false;
		m_whiteTexture                       = make_reference<gpu::VKTexture2D>(m_ctx, white_texture_spec_info, &white_image_data, sizeof(uint32));

		for (uint32 i{0u}; i < 32; ++i)
			m_textureSlots[i] = m_whiteTexture;

		_createRenderTargetResources();
	}

	Renderer2D::~Renderer2D()
	{
		m_cameraUBs->unmapMemory();
		delete[] m_quadVertexBase;
	}

	void Renderer2D::begin(vk::raii::CommandBuffer &p_cmd, uint32 p_frame_index, const tsm::float4x4 &p_view_matrix, const tsm::float4x4 &p_proj_matrix)
	{
		CameraUB ubo{};
		ubo.view = p_view_matrix;
		ubo.proj = p_proj_matrix;

		ubo.proj[1][1] *= -1.0f;

		std::memcpy(m_mappedCameraUBs[p_frame_index], &ubo, sizeof(CameraUB));

		m_quadIndexCount = 0u;
		m_quadVertexPtr  = m_quadVertexBase;

		m_stats.quadCount = 0u;
	}

	void Renderer2D::end(vk::raii::CommandBuffer &p_cmd, uint32 p_frame_index)
	{
		m_ctx->transitionImageLayout(m_renderTargetTexture->getImage()->getImage(), vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eColorAttachmentOptimal,
									 vk::AccessFlagBits::eShaderRead, vk::AccessFlagBits::eColorAttachmentWrite, vk::PipelineStageFlagBits::eFragmentShader,
									 vk::PipelineStageFlagBits::eColorAttachmentOutput, 1, vk::ImageAspectFlagBits::eColor);

		vk::RenderingAttachmentInfo colour_attachment_info{};
		colour_attachment_info.imageView   = m_renderTargetTexture->getImage()->getImageView();
		colour_attachment_info.clearValue  = vk::ClearColorValue{1.0f, 1.0f, 0.0f, 1.0f};
		colour_attachment_info.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
		colour_attachment_info.loadOp      = vk::AttachmentLoadOp::eClear;
		colour_attachment_info.storeOp     = vk::AttachmentStoreOp::eStore;

		vk::RenderingAttachmentInfo depth_attachment_info{};
		depth_attachment_info.clearValue  = vk::ClearDepthStencilValue{1.0f, 0u};
		depth_attachment_info.imageView   = m_renderTargetDepthImage->getImageView();
		depth_attachment_info.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
		depth_attachment_info.loadOp      = vk::AttachmentLoadOp::eClear;
		depth_attachment_info.storeOp     = vk::AttachmentStoreOp::eStore;

		vk::RenderingInfo rendering_info{};
		rendering_info.renderArea           = vk::Rect2D{{0, 0}, {m_createInfo.renderTargetWidth, m_createInfo.renderTargetHeight}};
		rendering_info.layerCount           = 1;
		rendering_info.colorAttachmentCount = 1;
		rendering_info.pColorAttachments    = &colour_attachment_info;
		rendering_info.pDepthAttachment     = &depth_attachment_info;

		Renderer::beginRendering(rendering_info, p_cmd, p_frame_index, m_quadRenderPass);

		const auto size = static_cast<uint32>(reinterpret_cast<uint8 *>(m_quadVertexPtr) - reinterpret_cast<uint8 *>(m_quadVertexBase));
		if (size) // Apparently you have to check ts, or things won't work correctly and there will be artifacts...
		{
			m_quadVertexBuffer->setData(m_quadVertexBase, size, 0);

			for (uint32 i{0u}; i < m_textureSlots.size(); ++i)
			{
				// if (m_textureSlots[i])
				// m_quadMaterial->set("u_Textures", m_textureSlots[i], i);
				// else
				// m_quadMaterial->set("u_Textures", m_whiteTexture, i);
			}
			m_quadMaterial->set("u_WhiteTexture", m_whiteTexture);

			Renderer::renderGeometry(p_cmd, p_frame_index, m_quadPipeline, m_quadVertexBuffer, m_quadIndexBuffer, m_quadIndexCount, m_quadMaterial, glm::mat4{1.0f});
		}

		Renderer::endRendering(p_cmd);

		m_ctx->transitionImageLayout(m_renderTargetTexture->getImage()->getImage(), vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
									 vk::AccessFlagBits::eColorAttachmentWrite, vk::AccessFlagBits::eShaderRead, vk::PipelineStageFlagBits::eColorAttachmentOutput,
									 vk::PipelineStageFlagBits::eFragmentShader, 1, vk::ImageAspectFlagBits::eColor);
	}

	void Renderer2D::submitQuad(const tsm::float3 &p_position, const tsm::float2 &p_scale, const tsm::float4 &p_colour)
	{
		const tsm::float4x4 transform = glm::translate(glm::mat4{1.0f}, p_position) * glm::scale(glm::mat4{1.0f}, {p_scale.x, p_scale.y, 1.0f});
		submitQuad(transform, p_colour);
	}

	void Renderer2D::submitQuad(const tsm::float2 &p_position, const tsm::float2 &p_scale, const tsm::float4 &p_colour)
	{
		const tsm::float4x4 transform = glm::translate(glm::mat4{1.0f}, tsm::float3{p_position.x, p_position.y, 0.0f}) * glm::scale(glm::mat4{1.0f}, {
																																		p_scale.x,
																																		p_scale.y,
																																		1.0f
																																	});
		submitQuad(transform, p_colour);
	}

	void Renderer2D::submitQuad(const tsm::float4x4 &p_transform, const tsm::float4 &p_colour)
	{
		if (m_quadIndexCount >= m_maxIndices)
			_beginNewBatch();

		for (uint32 i{0u}; i < 4u; ++i)
		{
			m_quadVertexPtr->position = p_transform * m_quadVertexPositions[i];
			m_quadVertexPtr->colour   = p_colour;
			m_quadVertexPtr->texCoord = m_quadVertexTexCoords[i];
			m_quadVertexPtr++;
		}
		m_quadIndexCount += 6u;
		m_stats.quadCount++;
	}

	const Renderer2D::Stats &Renderer2D::getStats() const
	{
		return m_stats;
	}

	const RefPtr<gpu::VKTexture2D> &Renderer2D::getColourOutput() const
	{
		return m_renderTargetTexture;
	}

	void Renderer2D::onResize(uint32 p_width, uint32 p_height)
	{
		m_createInfo.renderTargetWidth  = p_width;
		m_createInfo.renderTargetHeight = p_height;

		m_renderTargetTexture->resize(p_width, p_height);
		m_renderTargetDepthImage->resize(p_width, p_height);
	}

	void Renderer2D::_beginNewBatch()
	{
		// end();

		m_quadIndexCount = 0u;
		m_quadVertexPtr  = m_quadVertexBase;
	}

	void Renderer2D::_createRenderTargetResources()
	{
		#if 0
		m_renderTargetImage               = nullptr; m_renderTargetImageMemory = nullptr; m_renderTargetImageView = nullptr; m_renderTargetImageSampler = nullptr;
		m_renderTargetDescriptorImageInfo = vk::DescriptorImageInfo{}; vk::Format image_format = vk::Format::eR8G8B8A8Srgb; m_ctx->
				createImage(m_createInfo.renderTargetWidth, m_createInfo.renderTargetHeight, 1, vk::SampleCountFlagBits::e1, image_format, vk::ImageTiling::eOptimal,
							vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, m_renderTargetImage,
							m_renderTargetImageMemory); m_ctx->
				transitionImageLayout(m_renderTargetImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal, vk::AccessFlagBits::eNone,
									  vk::AccessFlagBits::eShaderRead, vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eFragmentShader, 1,
									  vk::ImageAspectFlagBits::eColor);
		m_renderTargetImageView = m_ctx->createImageView(m_renderTargetImage, image_format, vk::ImageAspectFlagBits::eColor, 1); auto physical_device_props = m_ctx->
				getPhysicalDevice().getProperties(); vk::SamplerCreateInfo sampler_create_info{}; sampler_create_info.addressModeU = vk::SamplerAddressMode::eRepeat;
		sampler_create_info.addressModeV = vk::SamplerAddressMode::eRepeat; sampler_create_info.addressModeW = vk::SamplerAddressMode::eRepeat;
		sampler_create_info.magFilter = vk::Filter::eLinear; sampler_create_info.minFilter = vk::Filter::eLinear;
		sampler_create_info.mipmapMode = vk::SamplerMipmapMode::eLinear; sampler_create_info.addressModeU = vk::SamplerAddressMode::eRepeat;
		sampler_create_info.addressModeV = vk::SamplerAddressMode::eRepeat; sampler_create_info.addressModeW = vk::SamplerAddressMode::eRepeat;
		sampler_create_info.mipLodBias = 0.0f; sampler_create_info.anisotropyEnable = true;
		sampler_create_info.maxAnisotropy = physical_device_props.limits.maxSamplerAnisotropy; sampler_create_info.compareEnable = false;
		sampler_create_info.compareOp = vk::CompareOp::eAlways; sampler_create_info.minLod = 0.0f; sampler_create_info.maxLod = vk::LodClampNone;
		sampler_create_info.borderColor = vk::BorderColor::eFloatCustomEXT; sampler_create_info.unnormalizedCoordinates = false;

		// This is purely aesthetic
		vk::SamplerCustomBorderColorCreateInfoEXT border_colour_create_info{}; border_colour_create_info.customBorderColor = vk::ClearColorValue{1.0f, 0.0f, 1.0f, 1.0f};
		border_colour_create_info.format = vk::Format::eR8G8B8A8Srgb; sampler_create_info.pNext = &border_colour_create_info;
		m_renderTargetImageSampler = {m_ctx->getDevice(), sampler_create_info}; m_renderTargetDescriptorImageInfo = vk::DescriptorImageInfo{};
		m_renderTargetDescriptorImageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal; m_renderTargetDescriptorImageInfo.imageView = m_renderTargetImageView;
		m_renderTargetDescriptorImageInfo.sampler = m_renderTargetImageSampler;
		#endif
	}
}
