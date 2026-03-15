#include "renderer_3d.hpp"

#include "globals.hpp"
#include "render_command.hpp"

namespace toaster
{
	Renderer3D::Renderer3D(const Renderer3DCreateInfo &p_create_info) : m_createInfo(p_create_info), m_maxVertices(p_create_info.maxCubes * 4u),
																		m_maxIndices(p_create_info.maxCubes * 6u)
	{
		m_cubeVertexBuffer = gpu::VertexBuffer::create(m_maxVertices * sizeof(CubeVertex));
		const auto vbl     = gpu::VertexBufferLayout{
			{gpu::EShaderDataType::eFloat4, "a_Position"},
			{gpu::EShaderDataType::eFloat4, "a_Colour"},
			{gpu::EShaderDataType::eFloat2, "a_TexCoord"},
			{gpu::EShaderDataType::eFloat, "a_TexIndex"}
		};
		m_cubeVertexBuffer->setLayout(vbl);

		m_cubeVertexBase = new CubeVertex[m_maxVertices];

		auto *cube_indices = new uint32[m_maxIndices];

		uint32 offset{0u};
		for (uint32 i{0u}; i < m_maxIndices; i += 6u)
		{
			cube_indices[i]     = offset;
			cube_indices[i + 1] = offset + 1;
			cube_indices[i + 2] = offset + 2;

			cube_indices[i + 3] = offset + 2;
			cube_indices[i + 4] = offset + 3;
			cube_indices[i + 5] = offset;

			offset += 4u;
		}
		m_cubeIndexBuffer = gpu::IndexBuffer::create(cube_indices, m_maxIndices);
		delete[] cube_indices;

		m_cubeVertexArray = gpu::VertexArray::create();
		m_cubeVertexArray->addVertexBuffer(m_cubeVertexBuffer);
		m_cubeVertexArray->setIndexBuffer(m_cubeIndexBuffer);

		m_whiteTexture            = gpu::Texture2D::create(1, 1);
		uint32 white_texture_data = 0xffffffff;
		m_whiteTexture->setData(&white_texture_data, sizeof(uint32));

		int samplers[c_maxTextureSlots];
		for (int i{0}; i < c_maxTextureSlots; ++i)
		{
			samplers[i] = i;
		}

		const auto cube_shader = Globals::shaderLibrary()->get("Cube");
		cube_shader->bind();
		cube_shader->setUniform("u_Textures", samplers, c_maxTextureSlots);

		m_textureSlots[0] = m_whiteTexture;

		m_cubeVertexPositions = {
			tsm::float4{-0.5f, -0.5f, 0.0f, 1.0f},
			tsm::float4{0.5f, -0.5f, 0.0f, 1.0f},
			tsm::float4{0.5f, 0.5f, 0.0f, 1.0f},
			tsm::float4{-0.5f, 0.5f, 0.0f, 1.0f}
		};

		m_cubeVertexTexCoords = {tsm::float2{0.0f, 0.0f}, tsm::float2{1.0f, 0.0f}, tsm::float2{1.0f, 1.0f}, tsm::float2{0.0f, 1.0f}};
	}

	Renderer3D::~Renderer3D()
	{
		delete[] m_cubeVertexBase;
	}

	void Renderer3D::begin(const tsm::float4x4 &p_view_matrix, const tsm::float4x4 &p_proj_matrix)
	{
		const auto cube_shader = Globals::shaderLibrary()->get("Cube");
		cube_shader->bind();

		cube_shader->setUniform("u_View", p_view_matrix);
		cube_shader->setUniform("u_Proj", p_proj_matrix);

		m_cubeIndexCount   = 0u;
		m_cubeVertexPtr    = m_cubeVertexBase;
		m_textureSlotIndex = 1u;
	}

	void Renderer3D::end()
	{
		const auto size = static_cast<uint32>(reinterpret_cast<uint8 *>(m_cubeVertexPtr) - reinterpret_cast<uint8 *>(m_cubeVertexBase));
		m_cubeVertexBuffer->setData(m_cubeVertexBase, size);

		for (uint32 i{0u}; i < m_textureSlotIndex; ++i)
		{
			m_textureSlots[i]->bind(i);
		}

		RenderCommand::drawIndexed(m_cubeVertexArray, m_cubeIndexCount);
	}

	void Renderer3D::submitCube(const tsm::float3 &p_position, const tsm::float2 &p_scale, const tsm::float4 &p_colour)
	{
		if (m_cubeIndexCount >= m_maxIndices)
			_beginNewBatch();

		const tsm::float4x4 transform = glm::translate(glm::mat4{1.0f}, p_position) * glm::scale(glm::mat4{1.0f}, {p_scale.x, p_scale.y, 1.0f});

		for (uint32 i{0u}; i < 4u; ++i)
		{
			m_cubeVertexPtr->position = transform * m_cubeVertexPositions[i];
			m_cubeVertexPtr->colour   = p_colour;
			m_cubeVertexPtr->texCoord = m_cubeVertexTexCoords[i];
			m_cubeVertexPtr->texIndex = 0.0f;
			m_cubeVertexPtr++;
		}

		m_cubeIndexCount += 6u;
	}

	void Renderer3D::submitCube(const tsm::float2 &p_position, const tsm::float2 &p_scale, const tsm::float4 &p_colour)
	{
		submitCube(tsm::float3{p_position.x, p_position.y, 0.0f}, p_scale, p_colour);
	}

	void Renderer3D::submitCube(const tsm::float3 &p_position, const tsm::float2 &p_scale, const RefPtr<gpu::Texture2D> &p_texture, const tsm::float4 &p_tint_colour)
	{
		if (m_cubeIndexCount >= m_maxIndices)
			_beginNewBatch();

		const tsm::float4x4 transform = glm::translate(glm::mat4{1.0f}, p_position) * glm::scale(glm::mat4{1.0f}, {p_scale.x, p_scale.y, 1.0f});

		const auto texture_index = static_cast<float32>(_getTextureSlotIndex(p_texture));

		for (uint32 i{0u}; i < 4u; ++i)
		{
			m_cubeVertexPtr->position = transform * m_cubeVertexPositions[i];
			m_cubeVertexPtr->colour   = p_tint_colour;
			m_cubeVertexPtr->texCoord = m_cubeVertexTexCoords[i];
			m_cubeVertexPtr->texIndex = texture_index;
			m_cubeVertexPtr++;
		}

		m_cubeIndexCount += 6u;
	}

	void Renderer3D::submitCube(const tsm::float2 &p_position, const tsm::float2 &p_scale, const RefPtr<gpu::Texture2D> &p_texture, const tsm::float4 &p_tint_colour)
	{
		submitCube(tsm::float3{p_position.x, p_position.y, 0.0f}, p_scale, p_texture, p_tint_colour);
	}

	void Renderer3D::_beginNewBatch()
	{
		end();

		m_cubeIndexCount   = 0u;
		m_cubeVertexPtr    = m_cubeVertexBase;
		m_textureSlotIndex = 1u;
	}

	uint32 Renderer3D::_getTextureSlotIndex(const RefPtr<gpu::Texture2D> &p_texture)
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
