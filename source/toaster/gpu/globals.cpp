#include "globals.hpp"

#include "io/filesystem.hpp"

namespace toaster::gpu
{
	struct GlobalData
	{
		RefPtr<Shader> g_quadShader;

		struct
		{
			RefPtr<VertexBuffer> vertexBuffer;
			RefPtr<IndexBuffer>  indexBuffer;
			RefPtr<VertexArray>  vertexArray;
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

		s_globalData->g_quadShader = Shader::create("Quad", {
														{EShaderType::eVertex, io::filesystem::readFile("quad.vert.glsl").c_str()},
														{EShaderType::ePixel, io::filesystem::readFile("planet.pixel.glsl").c_str()}
													});

		{
			std::vector<QuadVertex> vertices = {
				{{0.5f, 0.5f, 0.0f}, {1.0f, 1.0f}},
				{{0.5f, -0.5f, 0.0f}, {1.0f, 0.0f}},
				{{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f}},
				{{-0.5f, 0.5f, 0.0f}, {0.0f, 1.0f}},
			};
			std::vector<uint32> indices = {0, 1, 3, 1, 2, 3};

			s_globalData->g_quad.vertexBuffer = VertexBuffer::create(vertices.data(), vertices.size() * sizeof(QuadVertex));
			const auto vbl                    = VertexBufferLayout{{EShaderDataType::eFloat3, "a_Position"}, {EShaderDataType::eFloat2, "a_TexCoord"}};
			s_globalData->g_quad.vertexBuffer->setLayout(vbl);

			s_globalData->g_quad.indexBuffer = IndexBuffer::create(indices.data(), indices.size());

			s_globalData->g_quad.vertexArray = VertexArray::create();
			s_globalData->g_quad.vertexArray->addVertexBuffer(s_globalData->g_quad.vertexBuffer);
			s_globalData->g_quad.vertexArray->setIndexBuffer(s_globalData->g_quad.indexBuffer);
		}
	}

	void Globals::shutdown()
	{
		delete s_globalData;
	}

	RefPtr<Shader> Globals::quadShader()
	{
		return s_globalData->g_quadShader;
	}

	RefPtr<VertexArray> Globals::quadVertexArray()
	{
		return s_globalData->g_quad.vertexArray;
	}
}
