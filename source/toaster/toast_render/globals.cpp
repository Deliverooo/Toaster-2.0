#include "globals.hpp"

#include "toast_gpu/vk/vk_logical_device.hpp"
#include "toast_lib/io/filesystem.hpp"

namespace toaster
{
	struct GlobalData
	{
		gpu::VKLogicalDevice *device{nullptr};

		ShaderLibrary shaderLibrary;

		RefPtr<gpu::VKVertexBuffer> quadVertexBuffer{nullptr};
		RefPtr<gpu::VKIndexBuffer>  quadIndexBuffer{nullptr};

		std::vector<Globals::QuadVertex> quadVertices;
		std::vector<uint32>              quadIndices;

		RefPtr<gpu::VKTexture2D> whiteTexture{nullptr};
	};

	static GlobalData *s_globalData{nullptr};

	auto Globals::init(gpu::VKLogicalDevice *p_device, const io::filesystem::Path &p_binary_dir) -> void
	{
		s_globalData         = new GlobalData{};
		s_globalData->device = p_device;

		InitialiserList shader_stages = {vk::ShaderStageFlagBits::eVertex, vk::ShaderStageFlagBits::eFragment};
		{
			gpu::VKShader::Bytecode vs_bytecode{io::filesystem::readBinary(p_binary_dir / "shaders/depth-pre.vert.glsl.spv")};
			gpu::VKShader::Bytecode ps_bytecode{io::filesystem::readBinary(p_binary_dir / "shaders/depth-pre.pixel.glsl.spv")};
			TST_ASSERT_MSG(!vs_bytecode.empty() && !ps_bytecode.empty(), "Failed to read shader file. Did you add it to the CMake compilation");
			InitialiserList bytecode = {vs_bytecode, ps_bytecode};
			const auto      depth_pre_shader{s_globalData->device->alloc<gpu::VKShader>(shader_stages, bytecode, "Depth-Pre")};
			s_globalData->shaderLibrary.add("Depth-Pre", depth_pre_shader); // Shader for geometry, not vk::ShaderStageFlagBits::eGeometry!
		}
		{
			gpu::VKShader::Bytecode vs_bytecode{io::filesystem::readBinary(p_binary_dir / "shaders/geometry.vert.glsl.spv")};
			gpu::VKShader::Bytecode ps_bytecode{io::filesystem::readBinary(p_binary_dir / "shaders/geometry.pixel.glsl.spv")};
			TST_ASSERT_MSG(!vs_bytecode.empty() && !ps_bytecode.empty(), "Failed to read shader file. Did you add it to the CMake compilation");
			std::initializer_list bytecode = {vs_bytecode, ps_bytecode};
			const auto            geometry_shader{s_globalData->device->alloc<gpu::VKShader>(shader_stages, bytecode, "Geometry")};
			s_globalData->shaderLibrary.add("Geometry", geometry_shader); // Shader for geometry, not vk::ShaderStageFlagBits::eGeometry!
		}
		{
			gpu::VKShader::Bytecode vs_bytecode{io::filesystem::readBinary(p_binary_dir / "shaders/composite.vert.glsl.spv")};
			gpu::VKShader::Bytecode ps_bytecode{io::filesystem::readBinary(p_binary_dir / "shaders/composite.pixel.glsl.spv")};
			TST_ASSERT_MSG(!vs_bytecode.empty() && !ps_bytecode.empty(), "Failed to read shader file. Did you add it to the CMake compilation");
			std::initializer_list<gpu::VKShader::Bytecode> bytecode = {vs_bytecode, ps_bytecode};
			const auto                                     composite_shader{s_globalData->device->alloc<gpu::VKShader>(shader_stages, bytecode, "Composite")};
			s_globalData->shaderLibrary.add("Composite", composite_shader);
		}
		{
			gpu::VKShader::Bytecode vs_bytecode{io::filesystem::readBinary(p_binary_dir / "shaders/skybox.vert.glsl.spv")};
			gpu::VKShader::Bytecode ps_bytecode{io::filesystem::readBinary(p_binary_dir / "shaders/skybox.pixel.glsl.spv")};
			TST_ASSERT_MSG(!vs_bytecode.empty() && !ps_bytecode.empty(), "Failed to read shader file. Did you add it to the CMake compilation");
			std::initializer_list<gpu::VKShader::Bytecode> bytecode = {vs_bytecode, ps_bytecode};
			const auto                                     skybox_shader{s_globalData->device->alloc<gpu::VKShader>(shader_stages, bytecode, "Skybox")};
			s_globalData->shaderLibrary.add("Skybox", skybox_shader);
		}
		{
			gpu::VKShader::Bytecode vs_bytecode{io::filesystem::readBinary(p_binary_dir / "shaders/quad.vert.glsl.spv")};
			gpu::VKShader::Bytecode ps_bytecode{io::filesystem::readBinary(p_binary_dir / "shaders/quad.pixel.glsl.spv")};
			TST_ASSERT_MSG(!vs_bytecode.empty() && !ps_bytecode.empty(), "Failed to read shader file. Did you add it to the CMake compilation");
			std::initializer_list<gpu::VKShader::Bytecode> bytecode = {vs_bytecode, ps_bytecode};
			const auto                                     quad_shader{s_globalData->device->alloc<gpu::VKShader>(shader_stages, bytecode, "Quad")};
			s_globalData->shaderLibrary.add("Quad", quad_shader);
		}

		{
			gpu::VKShader::Bytecode cs_bytecode{io::filesystem::readBinary(p_binary_dir / "shaders/test.comp.glsl.spv")};
			TST_ASSERT_MSG(!cs_bytecode.empty(), "Failed to read shader file. Did you add it to the CMake compilation");
			std::initializer_list<gpu::VKShader::Bytecode> bytecode = {cs_bytecode};
			const auto                                     compute_test_shader{s_globalData->device->alloc<gpu::VKShader>(shader_stages, bytecode, "Compute-Test")};
			s_globalData->shaderLibrary.add("Compute-Test", compute_test_shader);
		}

		s_globalData->quadVertices.emplace_back(QuadVertex{{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}});
		s_globalData->quadVertices.emplace_back(QuadVertex{{1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}});
		s_globalData->quadVertices.emplace_back(QuadVertex{{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}});
		s_globalData->quadVertices.emplace_back(QuadVertex{{-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}});

		s_globalData->quadIndices = {0, 1, 3, 1, 2, 3};

		vk::DeviceSize vbo_size{s_globalData->quadVertices.size() * sizeof(QuadVertex)};
		s_globalData->quadVertexBuffer = s_globalData->device->alloc<gpu::VKVertexBuffer>(s_globalData->quadVertices.data(), vbo_size);

		vk::DeviceSize ibo_size{s_globalData->quadIndices.size() * sizeof(uint32)};
		s_globalData->quadIndexBuffer = s_globalData->device->alloc<gpu::VKIndexBuffer>(s_globalData->quadIndices.data(), ibo_size);

		gpu::TextureSpecInfo white_texture_spec_info{};
		white_texture_spec_info.width  = 1;
		white_texture_spec_info.height = 1;
		white_texture_spec_info.format = vk::Format::eR8G8B8A8Unorm;
		uint32 white_texture_data{0xFFFFFFFF};
		s_globalData->whiteTexture = s_globalData->device->alloc<gpu::VKTexture2D>(white_texture_spec_info, &white_texture_data, sizeof(uint32));
	}

	auto Globals::shutdown() -> void
	{
		s_globalData->device = nullptr;
		delete s_globalData;
		s_globalData = nullptr;
	}

	auto Globals::getShaderLibrary() -> const ShaderLibrary &
	{
		TST_ASSERT_MSG(s_globalData, "Did you forget to initialise Globals?!");
		return s_globalData->shaderLibrary;
	}

	auto Globals::getFullscreenQuadVertexBuffer() -> const RefPtr<gpu::VKVertexBuffer> &
	{
		TST_ASSERT_MSG(s_globalData, "Did you forget to initialise Globals?!");
		return s_globalData->quadVertexBuffer;
	}

	auto Globals::getFullscreenQuadIndexBuffer() -> const RefPtr<gpu::VKIndexBuffer> &
	{
		TST_ASSERT_MSG(s_globalData, "Did you forget to initialise Globals?!");
		return s_globalData->quadIndexBuffer;
	}

	auto Globals::getFullscreenQuadVertices() -> const std::vector<QuadVertex> &
	{
		TST_ASSERT_MSG(s_globalData, "Did you forget to initialise Globals?!");
		return s_globalData->quadVertices;
	}

	auto Globals::getFullscreenQuadIndices() -> const std::vector<uint32> &
	{
		TST_ASSERT_MSG(s_globalData, "Did you forget to initialise Globals?!");
		return s_globalData->quadIndices;
	}

	auto Globals::getWhiteTexture() -> const RefPtr<gpu::VKTexture2D> &
	{
		TST_ASSERT_MSG(s_globalData, "Did you forget to initialise Globals?!");
		return s_globalData->whiteTexture;
	}
}
