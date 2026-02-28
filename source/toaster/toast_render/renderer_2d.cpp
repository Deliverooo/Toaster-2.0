#include "renderer_2d.hpp"

#include "globals.hpp"
#include "render_command.hpp"

namespace toaster
{
	Renderer2D::Renderer2D()
	{
		std::vector<QuadVertex> vertices = {
			{{0.5f, 0.5f, 0.0f}, {1.0f, 1.0f}},
			{{0.5f, -0.5f, 0.0f}, {1.0f, 0.0f}},
			{{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f}},
			{{-0.5f, 0.5f, 0.0f}, {0.0f, 1.0f}},
		};
		std::vector<uint32> indices = {0, 1, 3, 1, 2, 3};

		m_quadVertexBuffer = gpu::VertexBuffer::create(vertices.data(), vertices.size() * sizeof(QuadVertex));
		const auto vbl     = gpu::VertexBufferLayout{{gpu::EShaderDataType::eFloat3, "a_Position"}, {gpu::EShaderDataType::eFloat2, "a_TexCoord"}};
		m_quadVertexBuffer->setLayout(vbl);

		m_quadIndexBuffer = gpu::IndexBuffer::create(indices.data(), indices.size());

		m_quadVertexArray = gpu::VertexArray::create();
		m_quadVertexArray->addVertexBuffer(m_quadVertexBuffer);
		m_quadVertexArray->setIndexBuffer(m_quadIndexBuffer);
	}

	Renderer2D::~Renderer2D()
	{
	}

	void Renderer2D::begin(const tsm::float4x4 &p_view_matrix, const tsm::float4x4 &p_proj_matrix)
	{
		auto quad_shader = Globals::shaderLibrary()->get("Quad");
		quad_shader->bind();

		quad_shader->setUniform("u_View", p_view_matrix);
		quad_shader->setUniform("u_Proj", p_proj_matrix);
	}

	void Renderer2D::end()
	{
	}

	void Renderer2D::submitQuad(const tsm::float3 &p_position, const tsm::float2 &p_scale, const tsm::float4 &p_colour)
	{
		tsm::float4x4 model_matrix = glm::translate(glm::scale(tsm::float4x4{1.0f}, {p_scale.x, p_scale.y, 1.0f}), p_position);

		auto quad_shader = Globals::shaderLibrary()->get("Quad");
		quad_shader->setUniform("u_Model", model_matrix);

		quad_shader->setUniform("u_Colour", p_colour);

		RenderCommand::drawIndexed(m_quadVertexArray);
	}

	void Renderer2D::submitQuad(const tsm::float2 &p_position, const tsm::float2 &p_scale, const tsm::float4 &p_colour)
	{
		submitQuad(tsm::float3{p_position.x, p_position.y, 0.0f}, p_scale, p_colour);
	}
}
