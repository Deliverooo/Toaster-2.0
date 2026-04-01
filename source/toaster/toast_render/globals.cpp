#include "globals.hpp"

#include "toast_lib/io/filesystem.hpp"

namespace toaster
{
	struct GlobalData
	{
		RefPtr<ShaderLibrary> g_shaderLibrary;

		RefPtr<gpu::IVertexBuffer> g_quadVertexBuffer;
		RefPtr<gpu::IIndexBuffer>  g_quadIndexBuffer;
		RefPtr<gpu::IVertexArray>  g_quadVertexArray;

		RefPtr<gpu::IVertexBuffer> g_fullscreenQuadVertexBuffer;
		RefPtr<gpu::IVertexArray>  g_fullscreenQuadVertexArray;
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

		const io::filesystem::Path shader_dir = "../source/toaster/toast_shaders/";

		TST_ASSERT_MSG(io::filesystem::exists(shader_dir),
					   "You probably set the working directory before creating the application, or the .exe is not in the bin directory");

		s_globalData->g_shaderLibrary = make_reference<ShaderLibrary>();

		const auto quad_shader = gpu::IShader::create("Quad", {
														  {gpu::EShaderType::eVertex, io::filesystem::readFile(shader_dir / "quad.vert.glsl").c_str()},
														  {gpu::EShaderType::ePixel, io::filesystem::readFile(shader_dir / "quad.pixel.glsl").c_str()}
													  });
		s_globalData->g_shaderLibrary->add("Quad", quad_shader);

		const auto fullscreen_quad_shader = gpu::IShader::create("Fullscreen_Quad", {
																	 {
																		 gpu::EShaderType::eVertex,
																		 io::filesystem::readFile(shader_dir / "fullscreen_quad.vert.glsl").c_str()
																	 },
																	 {
																		 gpu::EShaderType::ePixel,
																		 io::filesystem::readFile(shader_dir / "fullscreen_quad.pixel.glsl").c_str()
																	 }
																 });
		s_globalData->g_shaderLibrary->add("Fullscreen_Quad", fullscreen_quad_shader);

		const auto mesh_shader = gpu::IShader::create("Mesh", {
														  {gpu::EShaderType::eVertex, io::filesystem::readFile(shader_dir / "mesh.vert.glsl").c_str()},
														  {gpu::EShaderType::ePixel, io::filesystem::readFile(shader_dir / "mesh.pixel.glsl").c_str()}
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

			s_globalData->g_quadVertexBuffer = gpu::IVertexBuffer::create(vertices.data(), vertices.size() * sizeof(QuadVertex));
			const auto vbl                   = gpu::VertexBufferLayout{{gpu::EShaderDataType::eFloat3, "a_Position"}, {gpu::EShaderDataType::eFloat2, "a_TexCoords"}};
			s_globalData->g_quadVertexBuffer->setLayout(vbl);

			s_globalData->g_quadIndexBuffer = gpu::IIndexBuffer::create(indices.data(), indices.size());

			s_globalData->g_quadVertexArray = gpu::IVertexArray::create();
			s_globalData->g_quadVertexArray->addVertexBuffer(s_globalData->g_quadVertexBuffer);
			s_globalData->g_quadVertexArray->setIndexBuffer(s_globalData->g_quadIndexBuffer);
		}

		{
			std::vector<QuadVertex> vertices = {
				{{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
				{{1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
				{{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
				{{-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
			};
			std::vector<uint32> indices = {0, 1, 3, 1, 2, 3};

			s_globalData->g_fullscreenQuadVertexBuffer = gpu::IVertexBuffer::create(vertices.data(), vertices.size() * sizeof(QuadVertex));
			const auto vbl = gpu::VertexBufferLayout{{gpu::EShaderDataType::eFloat3, "a_Position"}, {gpu::EShaderDataType::eFloat2, "a_TexCoords"}};
			s_globalData->g_fullscreenQuadVertexBuffer->setLayout(vbl);

			s_globalData->g_fullscreenQuadVertexArray = gpu::IVertexArray::create();
			s_globalData->g_fullscreenQuadVertexArray->addVertexBuffer(s_globalData->g_fullscreenQuadVertexBuffer);
			s_globalData->g_fullscreenQuadVertexArray->setIndexBuffer(s_globalData->g_quadIndexBuffer);
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

	RefPtr<gpu::IVertexArray> Globals::quadVertexArray()
	{
		return s_globalData->g_quadVertexArray;
	}

	RefPtr<gpu::IVertexArray> Globals::fullscreenQuadVertexArray()
	{
		return s_globalData->g_fullscreenQuadVertexArray;
	}
}
