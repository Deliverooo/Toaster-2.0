#include "renderer_2d.hpp"

#include "globals.hpp"
#include "render_command.hpp"

namespace toaster
{
	Renderer2D::Renderer2D(const Renderer2DCreateInfo &p_create_info) : m_createInfo(p_create_info), m_maxVertices(p_create_info.maxQuads * 4u),
																		m_maxIndices(p_create_info.maxQuads * 6u)
	{
		m_quadVertexBuffer = gpu::IVertexBuffer::create(m_maxVertices * sizeof(QuadVertex));
		const auto vbl     = gpu::VertexBufferLayout{
			{gpu::EShaderDataType::eFloat4, "a_Position"},
			{gpu::EShaderDataType::eFloat4, "a_Colour"},
			{gpu::EShaderDataType::eFloat2, "a_TexCoord"},
			{gpu::EShaderDataType::eFloat, "a_TexIndex"}
		};
		m_quadVertexBuffer->setLayout(vbl);

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
		m_quadIndexBuffer = gpu::IIndexBuffer::create(quad_indices, m_maxIndices);
		delete[] quad_indices;

		m_quadVertexArray = gpu::IVertexArray::create();
		m_quadVertexArray->addVertexBuffer(m_quadVertexBuffer);
		m_quadVertexArray->setIndexBuffer(m_quadIndexBuffer);

		m_whiteTexture            = gpu::ITexture2D::create(1, 1);
		uint32 white_texture_data = 0xffffffff;
		m_whiteTexture->setData(&white_texture_data, sizeof(uint32));

		int samplers[c_maxTextureSlots];
		for (int i{0}; i < c_maxTextureSlots; ++i)
		{
			samplers[i] = i;
		}

		const auto quad_shader = Globals::shaderLibrary()->get("Quad");
		quad_shader->bind();
		quad_shader->setUniform("u_Textures", samplers, c_maxTextureSlots);

		m_textureSlots[0] = m_whiteTexture;

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
		delete[] m_quadVertexBase;
	}

	void Renderer2D::begin(const Camera &p_camera, const tsm::float4x4 &p_transform)
	{
		const auto quad_shader = Globals::shaderLibrary()->get("Quad");
		quad_shader->bind();

		quad_shader->setUniform("u_View", glm::inverse(p_transform));
		quad_shader->setUniform("u_Proj", p_camera.getProjectionMatrix());

		m_quadIndexCount   = 0u;
		m_quadVertexPtr    = m_quadVertexBase;
		m_textureSlotIndex = 1u;
	}

	void Renderer2D::begin(const tsm::float4x4 &p_view_matrix, const tsm::float4x4 &p_proj_matrix)
	{
		const auto quad_shader = Globals::shaderLibrary()->get("Quad");
		quad_shader->bind();

		quad_shader->setUniform("u_View", p_view_matrix);
		quad_shader->setUniform("u_Proj", p_proj_matrix);

		m_quadIndexCount   = 0u;
		m_quadVertexPtr    = m_quadVertexBase;
		m_textureSlotIndex = 1u;
	}

	void Renderer2D::end()
	{
		const auto size = static_cast<uint32>(reinterpret_cast<uint8 *>(m_quadVertexPtr) - reinterpret_cast<uint8 *>(m_quadVertexBase));
		m_quadVertexBuffer->setData(m_quadVertexBase, size);

		for (uint32 i{0u}; i < m_textureSlotIndex; ++i)
		{
			m_textureSlots[i]->bind(i);
		}

		RenderCommand::drawIndexed(m_quadVertexArray, m_quadIndexCount);
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
			m_quadVertexPtr->texIndex = 0.0f;
			m_quadVertexPtr++;
		}
		m_quadIndexCount += 6u;
	}

	void Renderer2D::submitQuad(const tsm::float3 &p_position, const tsm::float2 &p_scale, const RefPtr<gpu::ITexture2D> &p_texture, const tsm::float4 &p_tint_colour)
	{
		const tsm::float4x4 transform = glm::translate(glm::mat4{1.0f}, p_position) * glm::scale(glm::mat4{1.0f}, {p_scale.x, p_scale.y, 1.0f});
		submitQuad(transform, p_texture, p_tint_colour);
	}

	void Renderer2D::submitQuad(const tsm::float2 &p_position, const tsm::float2 &p_scale, const RefPtr<gpu::ITexture2D> &p_texture, const tsm::float4 &p_tint_colour)
	{
		const tsm::float4x4 transform = glm::translate(glm::mat4{1.0f}, tsm::float3{p_position.x, p_position.y, 0.0f}) * glm::scale(glm::mat4{1.0f}, {
																																		p_scale.x,
																																		p_scale.y,
																																		1.0f
																																	});
		submitQuad(transform, p_texture, p_tint_colour);
	}

	void Renderer2D::submitQuad(const tsm::float4x4 &p_transform, const RefPtr<gpu::ITexture2D> &p_texture, const tsm::float4 &p_tint_colour)
	{
		if (m_quadIndexCount >= m_maxIndices)
			_beginNewBatch();

		const auto texture_index = static_cast<float32>(_getTextureSlotIndex(p_texture));

		for (uint32 i{0u}; i < 4u; ++i)
		{
			m_quadVertexPtr->position = p_transform * m_quadVertexPositions[i];
			m_quadVertexPtr->colour   = p_tint_colour;
			m_quadVertexPtr->texCoord = m_quadVertexTexCoords[i];
			m_quadVertexPtr->texIndex = texture_index;
			m_quadVertexPtr++;
		}

		m_quadIndexCount += 6u;
	}

	void Renderer2D::_beginNewBatch()
	{
		end();

		m_quadIndexCount   = 0u;
		m_quadVertexPtr    = m_quadVertexBase;
		m_textureSlotIndex = 1u;
	}

	uint32 Renderer2D::_getTextureSlotIndex(const RefPtr<gpu::ITexture2D> &p_texture)
	{
		uint32 texture_index{0u};
		for (uint32 i{1u}; i < m_textureSlotIndex; ++i)
		{
			if (*m_textureSlots[i] == *p_texture)
			{
				texture_index = i;
				break;
			}
		}

		if (texture_index == 0u)
		{
			texture_index                      = m_textureSlotIndex;
			m_textureSlots[m_textureSlotIndex] = p_texture;
			m_textureSlotIndex++;
		}

		return texture_index;
	}
}
