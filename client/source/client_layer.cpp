#include "client_layer.hpp"

#include <iostream>
#include <openglhpp/opengl.hpp>

#include "io/file_stream.hpp"

namespace shaders::opengl
{
	#include "triangle.vert.glsl.spv.gl.inl"
	#include "triangle.pixel.glsl.spv.gl.inl"
}

namespace toaster
{
	float vertices[] = {
		0.5f,
		0.5f,
		0.0f,
		1.0f,
		1.0f,
		1.0f,
		0.5f,
		-0.5f,
		0.0f,
		0.0f,
		1.0f,
		0.0f,
		-0.5f,
		-0.5f,
		0.0f,
		0.0f,
		1.0f,
		1.0f,
		-0.5f,
		0.5f,
		0.0f,
		1.0f,
		1.0f,
		1.0f,
	};
	unsigned int indices[] = {
		// note that we start from 0!
		0,
		1,
		3,
		// first Triangle
		1,
		2,
		3 // second Triangle
	};

	ClientLayer::ClientLayer(Application *p_app_parent) : IAppLayer(p_app_parent)
	{
		io::filesystem::setWorkingDirectory("../../../source/toaster/toast_shaders");

		auto vs_source = io::filesystem::readFile("triangle.vert.glsl");
		auto ps_source = io::filesystem::readFile("triangle.pixel.glsl");

		std::map<gpu::EShaderType, const char *> shader_source_map = {{gpu::EShaderType::eVertex, vs_source.c_str()}, {gpu::EShaderType::ePixel, ps_source.c_str()}};
		m_shader                                                   = gpu::Shader::create("Triangle", shader_source_map);

		auto vertex_buffer = gpu::VertexBuffer::create(vertices, sizeof(vertices));
		vertex_buffer->setLayout({{gpu::EShaderDataType::eFloat3, "a_Position"}, {gpu::EShaderDataType::eFloat3, "a_Colour"}});

		auto index_buffer = gpu::IndexBuffer::create(indices, 6);
		m_vao             = gpu::VertexArray::create();
		m_vao->addVertexBuffer(vertex_buffer);
		m_vao->setIndexBuffer(index_buffer);
		m_vao->unbind();
	}

	ClientLayer::~ClientLayer()
	{
	}

	void ClientLayer::onInit()
	{
	}

	void ClientLayer::onDestroy()
	{
	}

	void ClientLayer::onUpdate(float32 p_dt)
	{
		gl::clear(gl::ClearMaskBits::eColor | gl::ClearMaskBits::eDepth);
		gl::clearColor(0.2f, 0.3f, 0.3f, 1.0f);

		m_shader->bind();

		m_shader->setUniform("u_Tst", 0.5f);
		m_vao->bind();

		gl::drawElements(gl::DrawMode::eTriangles, 6, gl::DataType::eUnsignedInt, nullptr);
	}

	void ClientLayer::onEvent(Event &p_event)
	{
	}
}
