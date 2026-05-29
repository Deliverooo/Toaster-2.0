#include "toast_render/globals.hpp"

#include "toast_gpu/vk/vk_logical_device.hpp"
#include "toast_lib/io/filesystem.hpp"

namespace toaster::render
{
	Globals::Globals(gpu::VKLogicalDevice *p_device, const io::filesystem::Path &p_binary_dir) : m_device(p_device), m_binaryDir(p_binary_dir)
	{
		InitialiserList shader_stages = {vk::ShaderStageFlagBits::eVertex, vk::ShaderStageFlagBits::eFragment};
		{
			gpu::VKShader::Bytecode vs_bytecode{io::filesystem::readBinary(p_binary_dir / "shaders/depth-pre.vert.glsl.spv")};
			gpu::VKShader::Bytecode ps_bytecode{io::filesystem::readBinary(p_binary_dir / "shaders/depth-pre.pixel.glsl.spv")};
			TST_ASSERT_MSG(!vs_bytecode.empty() && !ps_bytecode.empty(), "Failed to read shader file. Did you add it to the CMake compilation");
			InitialiserList bytecode = {vs_bytecode, ps_bytecode};
			const auto      depth_pre_shader{make_reference<gpu::VKShader>(m_device, shader_stages, bytecode, "Depth-Pre")};
			m_shaderLibrary.add("Depth-Pre", depth_pre_shader);
		}
		{
			gpu::VKShader::Bytecode cs_bytecode{io::filesystem::readBinary(p_binary_dir / "shaders/ambient_occlusion.comp.glsl.spv")};
			TST_ASSERT_MSG(!cs_bytecode.empty(), "Failed to read shader file. Did you add it to the CMake compilation");
			std::initializer_list bytecode = {cs_bytecode};
			auto                  stage    = {vk::ShaderStageFlagBits::eCompute};
			const auto            vbao_shader{make_reference<gpu::VKShader>(m_device, stage, bytecode, "Ambient_Occlusion")};
			m_shaderLibrary.add("Ambient_Occlusion", vbao_shader);
		}
		{
			gpu::VKShader::Bytecode vs_bytecode{io::filesystem::readBinary(p_binary_dir / "shaders/geometry.vert.glsl.spv")};
			gpu::VKShader::Bytecode ps_bytecode{io::filesystem::readBinary(p_binary_dir / "shaders/geometry.pixel.glsl.spv")};
			TST_ASSERT_MSG(!vs_bytecode.empty() && !ps_bytecode.empty(), "Failed to read shader file. Did you add it to the CMake compilation");
			std::initializer_list bytecode = {vs_bytecode, ps_bytecode};
			const auto            geometry_shader{make_reference<gpu::VKShader>(m_device, shader_stages, bytecode, "Geometry")};
			m_shaderLibrary.add("Geometry", geometry_shader); // Shader for geometry, not vk::ShaderStageFlagBits::eGeometry!
		}
		{
			gpu::VKShader::Bytecode vs_bytecode{io::filesystem::readBinary(p_binary_dir / "shaders/composite.vert.glsl.spv")};
			gpu::VKShader::Bytecode ps_bytecode{io::filesystem::readBinary(p_binary_dir / "shaders/composite.pixel.glsl.spv")};
			TST_ASSERT_MSG(!vs_bytecode.empty() && !ps_bytecode.empty(), "Failed to read shader file. Did you add it to the CMake compilation");
			std::initializer_list<gpu::VKShader::Bytecode> bytecode = {vs_bytecode, ps_bytecode};
			const auto                                     composite_shader{make_reference<gpu::VKShader>(m_device, shader_stages, bytecode, "Composite")};
			m_shaderLibrary.add("Composite", composite_shader);
		}
		{
			gpu::VKShader::Bytecode vs_bytecode{io::filesystem::readBinary(p_binary_dir / "shaders/skybox.vert.glsl.spv")};
			gpu::VKShader::Bytecode ps_bytecode{io::filesystem::readBinary(p_binary_dir / "shaders/skybox.pixel.glsl.spv")};
			TST_ASSERT_MSG(!vs_bytecode.empty() && !ps_bytecode.empty(), "Failed to read shader file. Did you add it to the CMake compilation");
			std::initializer_list<gpu::VKShader::Bytecode> bytecode = {vs_bytecode, ps_bytecode};
			const auto                                     skybox_shader{make_reference<gpu::VKShader>(m_device, shader_stages, bytecode, "Skybox")};
			m_shaderLibrary.add("Skybox", skybox_shader);
		}
		{
			gpu::VKShader::Bytecode vs_bytecode{io::filesystem::readBinary(p_binary_dir / "shaders/quad.vert.glsl.spv")};
			gpu::VKShader::Bytecode ps_bytecode{io::filesystem::readBinary(p_binary_dir / "shaders/quad.pixel.glsl.spv")};
			TST_ASSERT_MSG(!vs_bytecode.empty() && !ps_bytecode.empty(), "Failed to read shader file. Did you add it to the CMake compilation");
			std::initializer_list<gpu::VKShader::Bytecode> bytecode = {vs_bytecode, ps_bytecode};
			const auto                                     quad_shader{make_reference<gpu::VKShader>(m_device, shader_stages, bytecode, "Quad")};
			m_shaderLibrary.add("Quad", quad_shader);
		}
		{
			gpu::VKShader::Bytecode vs_bytecode{io::filesystem::readBinary(p_binary_dir / "shaders/anti-aliasing.vert.glsl.spv")};
			gpu::VKShader::Bytecode ps_bytecode{io::filesystem::readBinary(p_binary_dir / "shaders/anti-aliasing.pixel.glsl.spv")};
			TST_ASSERT_MSG(!vs_bytecode.empty() && !ps_bytecode.empty(), "Failed to read shader file. Did you add it to the CMake compilation");
			std::initializer_list<gpu::VKShader::Bytecode> bytecode = {vs_bytecode, ps_bytecode};
			const auto                                     anti_aliasing_shader{make_reference<gpu::VKShader>(m_device, shader_stages, bytecode, "Anti-Aliasing")};
			m_shaderLibrary.add("Anti-Aliasing", anti_aliasing_shader);
		}
		{
			gpu::VKShader::Bytecode vs_bytecode{io::filesystem::readBinary(p_binary_dir / "shaders/ssao.vert.glsl.spv")};
			gpu::VKShader::Bytecode ps_bytecode{io::filesystem::readBinary(p_binary_dir / "shaders/ssao.pixel.glsl.spv")};
			TST_ASSERT_MSG(!vs_bytecode.empty() && !ps_bytecode.empty(), "Failed to read shader file. Did you add it to the CMake compilation");
			std::initializer_list<gpu::VKShader::Bytecode> bytecode = {vs_bytecode, ps_bytecode};
			const auto                                     ssao_shader{make_reference<gpu::VKShader>(m_device, shader_stages, bytecode, "SSAO_Graphics")};
			m_shaderLibrary.add("SSAO_Graphics", ssao_shader);
		}
		{
			gpu::VKShader::Bytecode cs_bytecode{io::filesystem::readBinary(p_binary_dir / "shaders/test.comp.glsl.spv")};
			TST_ASSERT_MSG(!cs_bytecode.empty(), "Failed to read shader file. Did you add it to the CMake compilation");
			std::initializer_list<gpu::VKShader::Bytecode> bytecode = {cs_bytecode};
			auto                                           stage    = {vk::ShaderStageFlagBits::eCompute};
			const auto                                     compute_test_shader{make_reference<gpu::VKShader>(m_device, stage, bytecode, "Compute-Test")};
			m_shaderLibrary.add("Compute-Test", compute_test_shader);
		}
		{
			gpu::VKShader::Bytecode cs_bytecode{io::filesystem::readBinary(p_binary_dir / "shaders/equirectangular_to_cubemap.comp.glsl.spv")};
			TST_ASSERT_MSG(!cs_bytecode.empty(), "Failed to read shader file. Did you add it to the CMake compilation");
			std::initializer_list<gpu::VKShader::Bytecode> bytecode = {cs_bytecode};
			auto                                           stage    = {vk::ShaderStageFlagBits::eCompute};
			const auto                                     compute_test_shader{make_reference<gpu::VKShader>(m_device, stage, bytecode, "Equirectangular_To_CubeMap")};
			m_shaderLibrary.add("Equirectangular_To_CubeMap", compute_test_shader);
		}
		{
			gpu::VKShader::Bytecode cs_bytecode{io::filesystem::readBinary(p_binary_dir / "shaders/diffuse_irradiance_convolution.comp.glsl.spv")};
			TST_ASSERT_MSG(!cs_bytecode.empty(), "Failed to read shader file. Did you add it to the CMake compilation");
			std::initializer_list<gpu::VKShader::Bytecode> bytecode = {cs_bytecode};
			auto stage = {vk::ShaderStageFlagBits::eCompute};
			const auto compute_test_shader{make_reference<gpu::VKShader>(m_device, stage, bytecode, "Diffuse_Irradiance_Convolution")};
			m_shaderLibrary.add("Diffuse_Irradiance_Convolution", compute_test_shader);
		}
		{
			gpu::VKShader::Bytecode cs_bytecode{io::filesystem::readBinary(p_binary_dir / "shaders/ssao_blur.comp.glsl.spv")};
			TST_ASSERT_MSG(!cs_bytecode.empty(), "Failed to read shader file. Did you add it to the CMake compilation");
			std::initializer_list<gpu::VKShader::Bytecode> bytecode = {cs_bytecode};
			auto                                           stage    = {vk::ShaderStageFlagBits::eCompute};
			const auto                                     compute_test_shader{make_reference<gpu::VKShader>(m_device, stage, bytecode, "SSAO_Blur")};
			m_shaderLibrary.add("SSAO_Blur", compute_test_shader);
		}

		m_quadVertices.emplace_back(QuadVertex{{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}});
		m_quadVertices.emplace_back(QuadVertex{{1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}});
		m_quadVertices.emplace_back(QuadVertex{{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}});
		m_quadVertices.emplace_back(QuadVertex{{-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}});

		m_quadIndices = {0, 1, 3, 1, 2, 3};

		vk::DeviceSize vbo_size{m_quadVertices.size() * sizeof(QuadVertex)};
		m_quadVertexBuffer = make_reference<gpu::VKVertexBuffer>(m_device, m_quadVertices.data(), vbo_size);

		vk::DeviceSize ibo_size{m_quadIndices.size() * sizeof(uint32)};
		m_quadIndexBuffer = make_reference<gpu::VKIndexBuffer>(m_device, m_quadIndices.data(), ibo_size);

		gpu::TextureSpecInfo white_texture_spec_info{};
		white_texture_spec_info.width  = 1;
		white_texture_spec_info.height = 1;
		white_texture_spec_info.format = vk::Format::eR8G8B8A8Unorm;
		uint32 white_texture_data{0xFFFFFFFF};
		m_whiteTexture = make_reference<gpu::VKTexture2D>(m_device, white_texture_spec_info, &white_texture_data, sizeof(uint32));

		uint32 texture_3d_data[6]{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
		m_whiteTexture3D = make_reference<gpu::VKTexture3D>(m_device, white_texture_spec_info, Buffer{texture_3d_data, sizeof(uint32) * 6});
	}

	Globals::~Globals()
	{
	}

	auto Globals::shaderLibrary() const -> const ShaderLibrary &
	{
		return m_shaderLibrary;
	}

	auto Globals::fullscreenQuadVertexBuffer() const -> const gpu::VertexBufferHandle &
	{
		return m_quadVertexBuffer;
	}

	auto Globals::fullscreenQuadIndexBuffer() const -> const gpu::IndexBufferHandle &
	{
		return m_quadIndexBuffer;
	}

	auto Globals::fullscreenQuadVertices() const -> const std::vector<QuadVertex> &
	{
		return m_quadVertices;
	}

	auto Globals::fullscreenQuadIndices() const -> const std::vector<uint32> &
	{
		return m_quadIndices;
	}

	auto Globals::whiteTexture() const -> const gpu::Texture2DHandle &
	{
		return m_whiteTexture;
	}

	auto Globals::whiteTexture3D() const -> const gpu::Texture3DHandle &
	{
		return m_whiteTexture3D;
	}
}
