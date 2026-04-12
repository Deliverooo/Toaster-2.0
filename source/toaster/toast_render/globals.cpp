#include "globals.hpp"

#include "toast_gpu/vk/vk_gpu_context.hpp"
#include "toast_lib/io/filesystem.hpp"

namespace toaster
{
	struct GlobalData
	{
		gpu::VKGPUContext *ctx{nullptr};

		ShaderLibrary shaderLibrary;

		RefPtr<gpu::VKVertexBuffer> quadVertexBuffer{nullptr};
		RefPtr<gpu::VKIndexBuffer>  quadIndexBuffer{nullptr};

		std::vector<Globals::QuadVertex> quadVertices;
		std::vector<uint16>              quadIndices;

		RefPtr<gpu::VKTexture2D> whiteTexture{nullptr};
	};

	static GlobalData *s_globalData = nullptr;

	void Globals::init(gpu::VKGPUContext *p_ctx)
	{
		s_globalData      = new GlobalData{};
		s_globalData->ctx = p_ctx;

		{
			gpu::VKShader::Bytecode vs_bytecode = io::filesystem::readBinary("shaders/geometry.vert.glsl.spv");
			gpu::VKShader::Bytecode ps_bytecode = io::filesystem::readBinary("shaders/geometry.pixel.glsl.spv");
			TST_ASSERT_MSG(!vs_bytecode.empty() && !ps_bytecode.empty(), "Failed to read shader file. Did you add it to the CMake compilation");
			gpu::VKShader::BytecodeMap shader_bytecode_map{{vk::ShaderStageFlagBits::eVertex, vs_bytecode}, {vk::ShaderStageFlagBits::eFragment, ps_bytecode}};
			const auto                 geometry_shader{s_globalData->ctx->alloc<gpu::VKShader>(shader_bytecode_map)};
			s_globalData->shaderLibrary.add("Geometry", geometry_shader); // Shader for geometry, not vk::ShaderStageFlagBits::eGeometry!
		}
		{
			gpu::VKShader::Bytecode vs_bytecode = io::filesystem::readBinary("shaders/composite.vert.glsl.spv");
			gpu::VKShader::Bytecode ps_bytecode = io::filesystem::readBinary("shaders/composite.pixel.glsl.spv");
			TST_ASSERT_MSG(!vs_bytecode.empty() && !ps_bytecode.empty(), "Failed to read shader file. Did you add it to the CMake compilation");
			gpu::VKShader::BytecodeMap shader_bytecode_map{{vk::ShaderStageFlagBits::eVertex, vs_bytecode}, {vk::ShaderStageFlagBits::eFragment, ps_bytecode}};
			const auto                 composite_shader{s_globalData->ctx->alloc<gpu::VKShader>(shader_bytecode_map)};
			s_globalData->shaderLibrary.add("Composite", composite_shader);
		}
		{
			gpu::VKShader::Bytecode vs_bytecode = io::filesystem::readBinary("shaders/skybox.vert.glsl.spv");
			gpu::VKShader::Bytecode ps_bytecode = io::filesystem::readBinary("shaders/skybox.pixel.glsl.spv");
			TST_ASSERT_MSG(!vs_bytecode.empty() && !ps_bytecode.empty(), "Failed to read shader file. Did you add it to the CMake compilation");
			gpu::VKShader::BytecodeMap shader_bytecode_map{{vk::ShaderStageFlagBits::eVertex, vs_bytecode}, {vk::ShaderStageFlagBits::eFragment, ps_bytecode}};
			const auto                 skybox_shader{s_globalData->ctx->alloc<gpu::VKShader>(shader_bytecode_map)};
			s_globalData->shaderLibrary.add("Skybox", skybox_shader);
		}

		s_globalData->quadVertices.emplace_back(QuadVertex{{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}});
		s_globalData->quadVertices.emplace_back(QuadVertex{{1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}});
		s_globalData->quadVertices.emplace_back(QuadVertex{{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}});
		s_globalData->quadVertices.emplace_back(QuadVertex{{-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}});

		s_globalData->quadIndices = {0, 1, 3, 1, 2, 3};

		vk::DeviceSize vbo_size{s_globalData->quadVertices.size() * sizeof(QuadVertex)};
		s_globalData->quadVertexBuffer = s_globalData->ctx->alloc<gpu::VKVertexBuffer>(s_globalData->quadVertices.data(), vbo_size);

		vk::DeviceSize ibo_size{s_globalData->quadIndices.size() * sizeof(uint16)};
		s_globalData->quadIndexBuffer = s_globalData->ctx->alloc<gpu::VKIndexBuffer>(s_globalData->quadIndices.data(), ibo_size);

		gpu::TextureSpecInfo white_texture_spec_info{};
		white_texture_spec_info.width  = 1;
		white_texture_spec_info.height = 1;
		white_texture_spec_info.format = vk::Format::eR8G8B8A8Unorm;
		uint32 white_texture_data{0xFFFFFFFF};
		s_globalData->whiteTexture = s_globalData->ctx->alloc<gpu::VKTexture2D>(white_texture_spec_info, &white_texture_data, sizeof(uint32));
	}

	void Globals::shutdown()
	{
		s_globalData->ctx = nullptr;
		delete s_globalData;
	}

	const ShaderLibrary &Globals::getShaderLibrary()
	{
		return s_globalData->shaderLibrary;
	}

	const RefPtr<gpu::VKVertexBuffer> &Globals::getFullscreenQuadVertexBuffer()
	{
		return s_globalData->quadVertexBuffer;
	}

	const RefPtr<gpu::VKIndexBuffer> &Globals::getFullscreenQuadIndexBuffer()
	{
		return s_globalData->quadIndexBuffer;
	}

	const std::vector<Globals::QuadVertex> &Globals::getFullscreenQuadVertices()
	{
		return s_globalData->quadVertices;
	}

	const std::vector<uint16> &Globals::getFullscreenQuadIndices()
	{
		return s_globalData->quadIndices;
	}

	const RefPtr<gpu::VKTexture2D> &Globals::getWhiteTexture()
	{
		return s_globalData->whiteTexture;
	}
}
