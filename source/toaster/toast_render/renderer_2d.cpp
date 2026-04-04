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


		m_ctx->createImage(1280, 720, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eColorAttachment,
						   vk::MemoryPropertyFlagBits::eDeviceLocal, m_renderTargetImage, m_renderTargetImageMemory);
		m_renderTargetImageView = m_ctx->createImageView(m_renderTargetImage, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor);

		m_quadVertexBufferLayout = gpu::VertexBufferLayout{
			{gpu::EShaderDataType::eFloat4, "a_Position"},
			{gpu::EShaderDataType::eFloat4, "a_Colour"},
			{gpu::EShaderDataType::eFloat2, "a_TexCoord"},
		};

		vk::DeviceSize quad_vertex_buffer_size{sizeof(QuadVertex) * m_maxVertices};

		m_ctx->createBuffer(quad_vertex_buffer_size, vk::BufferUsageFlagBits::eTransferSrc,
							vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible, m_quadVertexBuffer, m_quadVertexBufferMemory);

		m_mappedQuadVertexBufferMemory = m_quadVertexBufferMemory.mapMemory(0, quad_vertex_buffer_size, {});

		m_quadVertexBase = new QuadVertex[m_maxVertices];

		auto *quad_indices = new uint32[m_maxIndices];

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

		vk::raii::Buffer       staging_buffer{nullptr};
		vk::raii::DeviceMemory staging_buffer_memory{nullptr};
		m_ctx->createBuffer(index_buffer_size, vk::BufferUsageFlagBits::eTransferSrc,
							vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, staging_buffer, staging_buffer_memory);

		void *data = staging_buffer_memory.mapMemory(0, index_buffer_size, {});
		std::memcpy(data, quad_indices, index_buffer_size);
		staging_buffer_memory.unmapMemory();

		m_ctx->createBuffer(index_buffer_size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal,
							m_quadIndexBuffer, m_quadIndexBufferMemory);
		m_ctx->copyBuffer(staging_buffer, m_quadIndexBuffer, index_buffer_size);

		delete[] quad_indices;

		m_quadVertexPositions = {
			tsm::float4{-0.5f, -0.5f, 0.0f, 1.0f},
			tsm::float4{0.5f, -0.5f, 0.0f, 1.0f},
			tsm::float4{0.5f, 0.5f, 0.0f, 1.0f},
			tsm::float4{-0.5f, 0.5f, 0.0f, 1.0f}
		};

		m_quadVertexTexCoords = {tsm::float2{0.0f, 0.0f}, tsm::float2{1.0f, 0.0f}, tsm::float2{1.0f, 1.0f}, tsm::float2{0.0f, 1.0f}};
	}

	Renderer2D::~Renderer2D()
	{
		m_quadVertexBufferMemory.unmapMemory();
		delete[] m_quadVertexBase;
	}

	void Renderer2D::begin(vk::raii::CommandBuffer& p_cmd, const tsm::float4x4 &p_view_matrix, const tsm::float4x4 &p_proj_matrix)
	{
		CameraUB ubo{};
		ubo.view  = p_view_matrix;
		ubo.proj  = p_proj_matrix;

		ubo.proj[1][1] *= -1.0f;

		// std::memcpy(m_mappedUniformBuffers[swapchain->getFrameIndex()], &ubo, sizeof(CameraUB));

		// const auto quad_shader = Globals::shaderLibrary()->get("Quad");
		// quad_shader->bind();
		//
		// quad_shader->setUniform("u_View", p_view_matrix);
		// quad_shader->setUniform("u_Proj", p_proj_matrix);

		m_quadIndexCount = 0u;
		m_quadVertexPtr  = m_quadVertexBase;

		m_stats.quadCount = 0u;
	}

	void Renderer2D::end(vk::raii::CommandBuffer& p_cmd)
	{
		vk::RenderingAttachmentInfo rendering_attachment_info{};
		rendering_attachment_info.imageView   = m_renderTargetImageView;
		rendering_attachment_info.clearValue  = vk::ClearColorValue{1.0f, 0.0f, 1.0f, 1.0f};
		rendering_attachment_info.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
		rendering_attachment_info.loadOp      = vk::AttachmentLoadOp::eClear;
		rendering_attachment_info.storeOp     = vk::AttachmentStoreOp::eStore;

		vk::RenderingInfo rendering_info{};
		rendering_info.colorAttachmentCount = 1;
		rendering_info.pColorAttachments    = &rendering_attachment_info;
		rendering_info.layerCount           = 1;
		rendering_info.renderArea           = {0, 0, 1280, 720};

		p_cmd.beginRendering(rendering_info);

		const auto size = static_cast<uint32>(reinterpret_cast<uint8 *>(m_quadVertexPtr) - reinterpret_cast<uint8 *>(m_quadVertexBase));
		if (size) // Apparently you have to check ts, or things won't work correctly and there will be artifacts...
		{
			std::memcpy(m_mappedQuadVertexBufferMemory, m_quadVertexBase, size);

			// RenderCommand::drawIndexed(m_quadVertexArray, m_quadIndexCount);
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

	void Renderer2D::_beginNewBatch()
	{
		// end();

		m_quadIndexCount = 0u;
		m_quadVertexPtr  = m_quadVertexBase;
	}
}
