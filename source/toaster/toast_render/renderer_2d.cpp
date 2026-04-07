#include "renderer_2d.hpp"

#include "globals.hpp"
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
		m_quadPipeline                          = make_reference<gpu::VKPipeline>(m_ctx, pipeline_create_info);

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
		m_quadIndexBuffer = make_reference<gpu::VKIndexBuffer>(m_ctx, index_buffer_size);

		delete[] quad_indices;

		m_quadVertexPositions = {
			tsm::float4{-0.5f, -0.5f, 0.0f, 1.0f},
			tsm::float4{0.5f, -0.5f, 0.0f, 1.0f},
			tsm::float4{0.5f, 0.5f, 0.0f, 1.0f},
			tsm::float4{-0.5f, 0.5f, 0.0f, 1.0f}
		};

		m_quadVertexTexCoords = {tsm::float2{0.0f, 0.0f}, tsm::float2{1.0f, 0.0f}, tsm::float2{1.0f, 1.0f}, tsm::float2{0.0f, 1.0f}};

		constexpr vk::DeviceSize ubo_size{sizeof(CameraUB)};
		m_uniformBuffers       = make_reference<gpu::VKUniformBufferPFF>(m_ctx, ubo_size, gpu::VKGPUContext::c_maxFramesInFlight);
		m_mappedUniformBuffers = m_uniformBuffers->mapMemory(ubo_size, 0);

		_createDescriptorPool();
		_createDescriptorSets();
		_createRenderTargetResources();
	}

	Renderer2D::~Renderer2D()
	{
		m_uniformBuffers->unmapMemory();
		delete[] m_quadVertexBase;
	}

	void Renderer2D::begin(vk::raii::CommandBuffer &p_cmd, uint32 p_frame_index, const tsm::float4x4 &p_view_matrix, const tsm::float4x4 &p_proj_matrix)
	{
		CameraUB ubo{};
		ubo.view = p_view_matrix;
		ubo.proj = p_proj_matrix;

		ubo.proj[1][1] *= -1.0f;

		std::memcpy(m_mappedUniformBuffers[p_frame_index], &ubo, sizeof(CameraUB));

		m_quadIndexCount = 0u;
		m_quadVertexPtr  = m_quadVertexBase;

		m_stats.quadCount = 0u;
	}

	void Renderer2D::end(vk::raii::CommandBuffer &p_cmd)
	{
		vk::RenderingAttachmentInfo colour_attachment_info{};
		colour_attachment_info.imageView   = m_renderTargetImageView;
		colour_attachment_info.clearValue  = vk::ClearColorValue{1.0f, 0.0f, 1.0f, 1.0f};
		colour_attachment_info.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
		colour_attachment_info.loadOp      = vk::AttachmentLoadOp::eClear;
		colour_attachment_info.storeOp     = vk::AttachmentStoreOp::eStore;

		vk::ClearValue              clear_depth = vk::ClearDepthStencilValue{1.0f, 0u};
		vk::RenderingAttachmentInfo depth_attachment_info{};
		depth_attachment_info.clearValue  = clear_depth;
		depth_attachment_info.imageView   = m_renderTargetDepthImageView;
		depth_attachment_info.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
		depth_attachment_info.loadOp      = vk::AttachmentLoadOp::eClear;
		depth_attachment_info.storeOp     = vk::AttachmentStoreOp::eStore;

		vk::RenderingInfo rendering_info{};
		rendering_info.renderArea           = vk::Rect2D{{0, 0}, {m_createInfo.renderTargetWidth, m_createInfo.renderTargetHeight}};
		rendering_info.layerCount           = 1;
		rendering_info.colorAttachmentCount = 1;
		rendering_info.pColorAttachments    = &colour_attachment_info;
		rendering_info.pDepthAttachment     = &depth_attachment_info;

		vk::Viewport viewport{};
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		viewport.x        = 0.0f;
		viewport.y        = 0.0f;
		viewport.width    = static_cast<float32>(m_createInfo.renderTargetWidth);
		viewport.height   = static_cast<float32>(m_createInfo.renderTargetHeight);

		vk::Rect2D scissor{vk::Offset2D{0, 0}, {m_createInfo.renderTargetWidth, m_createInfo.renderTargetHeight}};

		p_cmd.beginRendering(rendering_info);

		p_cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, m_quadPipeline->getPipeline());
		p_cmd.setViewport(0, viewport);
		p_cmd.setScissor(0, scissor);

		const auto size = static_cast<uint32>(reinterpret_cast<uint8 *>(m_quadVertexPtr) - reinterpret_cast<uint8 *>(m_quadVertexBase));
		if (size) // Apparently you have to check ts, or things won't work correctly and there will be artifacts...
		{
			m_quadVertexBuffer->setData(m_quadVertexBase, size, 0);

			m_quadVertexBuffer->bind(p_cmd);
			m_quadIndexBuffer->bind(p_cmd, vk::IndexType::eUint32);

			p_cmd.drawIndexed(m_quadIndexCount, 1, 0, 0, 0);
		}

		p_cmd.endRendering();
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

	vk::raii::Image &Renderer2D::getRenderTargetImage()
	{
		return m_renderTargetImage;
	}

	vk::raii::DeviceMemory &Renderer2D::getRenderTargetImageMemory()
	{
		return m_renderTargetImageMemory;
	}

	vk::raii::ImageView &Renderer2D::getRenderTargetImageView()
	{
		return m_renderTargetImageView;
	}

	void Renderer2D::onResize(uint32 p_width, uint32 p_height)
	{
		m_createInfo.renderTargetWidth  = p_width;
		m_createInfo.renderTargetHeight = p_height;
		_createRenderTargetResources();
	}

	void Renderer2D::_beginNewBatch()
	{
		// end();

		m_quadIndexCount = 0u;
		m_quadVertexPtr  = m_quadVertexBase;
	}

	void Renderer2D::_createRenderTargetResources()
	{
		m_renderTargetImage        = nullptr;
		m_renderTargetImageMemory  = nullptr;
		m_renderTargetImageView    = nullptr;
		m_renderTargetImageSampler = nullptr;

		m_renderTargetDescriptorImageInfo = vk::DescriptorImageInfo{};

		m_renderTargetDepthImage       = nullptr;
		m_renderTargetDepthImageMemory = nullptr;
		m_renderTargetDepthImageView   = nullptr;

		vk::Format image_format = vk::Format::eR8G8B8A8Srgb;
		m_ctx->createImage(m_createInfo.renderTargetWidth, m_createInfo.renderTargetHeight, 1, image_format, vk::ImageTiling::eOptimal,
						   vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, m_renderTargetImage,
						   m_renderTargetImageMemory);

		m_ctx->transitionImageLayout(m_renderTargetImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, vk::AccessFlagBits::eNone,
									 vk::AccessFlagBits::eColorAttachmentWrite, vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eColorAttachmentOutput,
									 1);

		m_renderTargetImageView = m_ctx->createImageView(m_renderTargetImage, image_format, vk::ImageAspectFlagBits::eColor, 1);

		auto physical_device_props = m_ctx->getPhysicalDevice().getProperties();

		vk::SamplerCreateInfo sampler_create_info{};
		sampler_create_info.addressModeU            = vk::SamplerAddressMode::eRepeat;
		sampler_create_info.addressModeV            = vk::SamplerAddressMode::eRepeat;
		sampler_create_info.addressModeW            = vk::SamplerAddressMode::eRepeat;
		sampler_create_info.magFilter               = vk::Filter::eLinear;
		sampler_create_info.minFilter               = vk::Filter::eLinear;
		sampler_create_info.mipmapMode              = vk::SamplerMipmapMode::eLinear;
		sampler_create_info.addressModeU            = vk::SamplerAddressMode::eRepeat;
		sampler_create_info.addressModeV            = vk::SamplerAddressMode::eRepeat;
		sampler_create_info.addressModeW            = vk::SamplerAddressMode::eRepeat;
		sampler_create_info.mipLodBias              = 0.0f;
		sampler_create_info.anisotropyEnable        = true;
		sampler_create_info.maxAnisotropy           = physical_device_props.limits.maxSamplerAnisotropy;
		sampler_create_info.compareEnable           = false;
		sampler_create_info.compareOp               = vk::CompareOp::eAlways;
		sampler_create_info.minLod                  = 0.0f;
		sampler_create_info.maxLod                  = vk::LodClampNone;
		sampler_create_info.borderColor             = vk::BorderColor::eFloatCustomEXT;
		sampler_create_info.unnormalizedCoordinates = false;

		// This is purely aesthetic
		vk::SamplerCustomBorderColorCreateInfoEXT border_colour_create_info{};
		border_colour_create_info.customBorderColor = vk::ClearColorValue{1.0f, 0.0f, 1.0f, 1.0f};
		border_colour_create_info.format            = vk::Format::eR8G8B8A8Srgb;

		sampler_create_info.pNext = &border_colour_create_info;

		m_renderTargetImageSampler = {m_ctx->getDevice(), sampler_create_info};

		m_renderTargetDescriptorImageInfo             = vk::DescriptorImageInfo{};
		m_renderTargetDescriptorImageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		m_renderTargetDescriptorImageInfo.imageView   = m_renderTargetImageView;
		m_renderTargetDescriptorImageInfo.sampler     = m_renderTargetImageSampler;

		vk::Format depth_format{m_ctx->findDepthFormat()};
		m_ctx->createImage(m_createInfo.renderTargetWidth, m_createInfo.renderTargetHeight, 1, depth_format, vk::ImageTiling::eOptimal,
						   vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, m_renderTargetDepthImage,
						   m_renderTargetDepthImageMemory);

		m_renderTargetDepthImageView = m_ctx->createImageView(m_renderTargetDepthImage, depth_format, vk::ImageAspectFlagBits::eDepth, 1);
	}

	void Renderer2D::_createDescriptorPool()
	{
		std::array<vk::DescriptorPoolSize, 2> descriptor_pool_sizes{};
		descriptor_pool_sizes[0].descriptorCount = 2 * gpu::VKGPUContext::c_maxFramesInFlight;
		descriptor_pool_sizes[0].type            = vk::DescriptorType::eUniformBuffer;

		descriptor_pool_sizes[1].descriptorCount = gpu::VKGPUContext::c_maxFramesInFlight;
		descriptor_pool_sizes[1].type            = vk::DescriptorType::eCombinedImageSampler;

		vk::DescriptorPoolCreateInfo descriptor_pool_create_info{};
		descriptor_pool_create_info.poolSizeCount = descriptor_pool_sizes.size();
		descriptor_pool_create_info.pPoolSizes    = descriptor_pool_sizes.data();
		descriptor_pool_create_info.maxSets       = gpu::VKGPUContext::c_maxFramesInFlight;

		m_descriptorPool = {m_ctx->getDevice(), descriptor_pool_create_info};
	}

	void Renderer2D::_createDescriptorSets()
	{
		{
			auto &                        set_layout = m_quadShader->getDescriptorSetLayout(0);
			std::vector                   descriptor_set_layouts{gpu::VKGPUContext::c_maxFramesInFlight, *set_layout};
			vk::DescriptorSetAllocateInfo descriptor_set_allocate_info{};
			descriptor_set_allocate_info.descriptorPool     = m_descriptorPool;
			descriptor_set_allocate_info.descriptorSetCount = gpu::VKGPUContext::c_maxFramesInFlight;
			descriptor_set_allocate_info.pSetLayouts        = descriptor_set_layouts.data();

			m_descriptorSets = m_ctx->getDevice().allocateDescriptorSets(descriptor_set_allocate_info);

			for (uint32 i{0u}; i < gpu::VKGPUContext::c_maxFramesInFlight; ++i)
			{
				std::array<vk::WriteDescriptorSet, 1> write_descriptor_sets{};
				write_descriptor_sets[0].descriptorCount = 1;
				write_descriptor_sets[0].descriptorType  = vk::DescriptorType::eUniformBuffer;
				write_descriptor_sets[0].pBufferInfo     = &m_uniformBuffers->getUBO(i)->getDescriptorInfo();
				write_descriptor_sets[0].dstSet          = m_descriptorSets[i];
				write_descriptor_sets[0].dstBinding      = 0;
				write_descriptor_sets[0].dstArrayElement = 0;

				m_ctx->getDevice().updateDescriptorSets(write_descriptor_sets, {});
			}
		}
	}
}
