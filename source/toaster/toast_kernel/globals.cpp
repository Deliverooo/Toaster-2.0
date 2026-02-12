#include "globals.hpp"

#include "io/filesystem.hpp"

namespace toaster
{
	struct GlobalData
	{
		RefPtr<ShaderLibrary> g_shaderLibrary;

		struct
		{
			RefPtr<gpu::VertexBuffer> vertexBuffer;
			RefPtr<gpu::IndexBuffer>  indexBuffer;
			RefPtr<gpu::VertexArray>  vertexArray;
		} g_quad;
	};

	struct QuadVertex
	{
		glm::vec3 position;
		glm::vec2 texCoord;
	};

	static GlobalData *s_globalData = nullptr;

	void Globals::init()
	{
		s_globalData = new GlobalData{};

		io::filesystem::setWorkingDirectory("../../../source/toaster/toast_shaders");

		s_globalData->g_shaderLibrary = make_reference<ShaderLibrary>();

		const auto quad_shader = gpu::Shader::create("Quad", {
														 {gpu::EShaderType::eVertex, io::filesystem::readFile("quad.vert.glsl").c_str()},
														 {gpu::EShaderType::ePixel, io::filesystem::readFile("quad.pixel.glsl").c_str()}
													 });
		s_globalData->g_shaderLibrary->add("Quad", quad_shader);

		auto mesh_shader = gpu::Shader::create("Mesh", {
												   {gpu::EShaderType::eVertex, io::filesystem::readFile("mesh.vert.glsl").c_str()},
												   {gpu::EShaderType::ePixel, io::filesystem::readFile("mesh.pixel.glsl").c_str()}
											   });
		s_globalData->g_shaderLibrary->add("Mesh", mesh_shader);

		{
			std::vector<QuadVertex> vertices = {
				{{0.5f, 0.5f, 0.0f}, {1.0f, 1.0f}},
				{{0.5f, -0.5f, 0.0f}, {1.0f, 0.0f}},
				{{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f}},
				{{-0.5f, 0.5f, 0.0f}, {0.0f, 1.0f}},
			};
			std::vector<uint32> indices = {0, 1, 3, 1, 2, 3};

			s_globalData->g_quad.vertexBuffer = gpu::VertexBuffer::create(vertices.data(), vertices.size() * sizeof(QuadVertex));
			const auto vbl                    = gpu::VertexBufferLayout{{gpu::EShaderDataType::eFloat3, "a_Position"}, {gpu::EShaderDataType::eFloat2, "a_TexCoord"}};
			s_globalData->g_quad.vertexBuffer->setLayout(vbl);

			s_globalData->g_quad.indexBuffer = gpu::IndexBuffer::create(indices.data(), indices.size());

			s_globalData->g_quad.vertexArray = gpu::VertexArray::create();
			s_globalData->g_quad.vertexArray->addVertexBuffer(s_globalData->g_quad.vertexBuffer);
			s_globalData->g_quad.vertexArray->setIndexBuffer(s_globalData->g_quad.indexBuffer);
		}
	}

	const RefPtr<ShaderLibrary> &Globals::shaderLibrary()
	{
		return s_globalData->g_shaderLibrary;
	}

	void Globals::shutdown()
	{
		delete s_globalData;
	}

	RefPtr<gpu::VertexArray> Globals::quadVertexArray()
	{
		return s_globalData->g_quad.vertexArray;
	}
}
