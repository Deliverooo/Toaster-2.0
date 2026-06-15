#include "toast_render/globals.hpp"

#include "toast_gpu/vk/vk_logical_device.hpp"
#include "toast_gpu/vk/vk_shader_compiler.hpp"
#include "toast_lib/io/filesystem.hpp"
#include "toast_render/render_context.hpp"

namespace toaster::render
{
	Globals::Globals(RenderContext &p_render_ctx, const io::filesystem::Path &p_binary_dir) : m_renderCtx(&p_render_ctx), m_binaryDir(p_binary_dir)
	{
		#pragma region shaders

		if (!io::filesystem::exists(m_binaryDir / "shaders/fullscreen_quad.vert.hlsl.spv"))
			TST_PERMA_ASSERT(false);

		// Fullscreen quad shaders
		m_dynamicShaderLibrary.add("Fullscreen_Quad_VS",
								   m_renderCtx->createShaderFromSpirV(m_binaryDir / "shaders/fullscreen_quad.vert.hlsl.spv", EShaderStage::eVertex,
																	  EShaderStage::ePixel));
		m_dynamicShaderLibrary.add("Fullscreen_Quad_PS",
								   m_renderCtx->createShaderFromSpirV(m_binaryDir / "shaders/fullscreen_quad.pixel.glsl.spv", EShaderStage::ePixel));

		// Quad shaders
		m_dynamicShaderLibrary.add("Quad_VS", m_renderCtx->createShaderFromSpirV(m_binaryDir / "shaders/quad_dynamic.vert.hlsl.spv", EShaderStage::eVertex,
																				 EShaderStage::ePixel));
		m_dynamicShaderLibrary.add("Quad_PS", m_renderCtx->createShaderFromSpirV(m_binaryDir / "shaders/quad_dynamic.pixel.glsl.spv", EShaderStage::ePixel));

		// Equirectangular to cube map shader
		m_dynamicShaderLibrary.add("Equirectangular_To_Cube_Map",
								   m_renderCtx->createShaderFromSpirV(m_binaryDir / "shaders/equirectangular_to_cube_map.comp.glsl.spv", EShaderStage::eCompute));

		// Depth pre shaders
		m_dynamicShaderLibrary.add("Depth_Pre_VS",
								   m_renderCtx->createShaderFromSpirV(m_binaryDir / "shaders/depth_pre.vert.hlsl.spv", EShaderStage::eVertex, EShaderStage::ePixel));
		m_dynamicShaderLibrary.add("Depth_Pre_PS", m_renderCtx->createShaderFromSpirV(m_binaryDir / "shaders/depth_pre.pixel.glsl.spv", EShaderStage::ePixel));

		// Skybox shaders
		m_dynamicShaderLibrary.add("Skybox_VS", m_renderCtx->createShaderFromSpirV(m_binaryDir / "shaders/skybox.vert.hlsl.spv", EShaderStage::eVertex,
																				   EShaderStage::ePixel));
		m_dynamicShaderLibrary.add("Skybox_PS", m_renderCtx->createShaderFromSpirV(m_binaryDir / "shaders/skybox.pixel.glsl.spv", EShaderStage::ePixel));

		#pragma endregion
		InitialiserList shader_stages = {vk::ShaderStageFlagBits::eVertex, vk::ShaderStageFlagBits::eFragment};
		{
			gpu::VKShader::Bytecode cs_bytecode{io::filesystem::readBinary(p_binary_dir / "shaders/ambient_occlusion.comp.glsl.spv")};
			TST_ASSERT_MSG(!cs_bytecode.empty(), "Failed to read shader file. Did you add it to the CMake compilation");
			std::initializer_list bytecode = {cs_bytecode};
			auto                  stage    = {vk::ShaderStageFlagBits::eCompute};
			const auto            vbao_shader{make_reference<gpu::VKShader>(m_renderCtx->getLogicalDevice(), stage, bytecode, "Ambient_Occlusion")};
			m_shaderLibrary.add("Ambient_Occlusion", vbao_shader);
		}
		{
			gpu::VKShader::Bytecode vs_bytecode{io::filesystem::readBinary(p_binary_dir / "shaders/geometry.vert.glsl.spv")};
			gpu::VKShader::Bytecode ps_bytecode{io::filesystem::readBinary(p_binary_dir / "shaders/geometry.pixel.glsl.spv")};
			TST_ASSERT_MSG(!vs_bytecode.empty() && !ps_bytecode.empty(), "Failed to read shader file. Did you add it to the CMake compilation");
			std::initializer_list bytecode = {vs_bytecode, ps_bytecode};
			const auto            geometry_shader{make_reference<gpu::VKShader>(m_renderCtx->getLogicalDevice(), shader_stages, bytecode, "Geometry")};
			m_shaderLibrary.add("Geometry", geometry_shader); // Shader for geometry, not vk::ShaderStageFlagBits::eGeometry!
		}
		{
			gpu::VKShader::Bytecode vs_bytecode{io::filesystem::readBinary(p_binary_dir / "shaders/composite.vert.glsl.spv")};
			gpu::VKShader::Bytecode ps_bytecode{io::filesystem::readBinary(p_binary_dir / "shaders/composite.pixel.glsl.spv")};
			TST_ASSERT_MSG(!vs_bytecode.empty() && !ps_bytecode.empty(), "Failed to read shader file. Did you add it to the CMake compilation");
			std::initializer_list<gpu::VKShader::Bytecode> bytecode = {vs_bytecode, ps_bytecode};
			const auto composite_shader{make_reference<gpu::VKShader>(m_renderCtx->getLogicalDevice(), shader_stages, bytecode, "Composite")};
			m_shaderLibrary.add("Composite", composite_shader);
		}
		{
			gpu::VKShader::Bytecode vs_bytecode{io::filesystem::readBinary(p_binary_dir / "shaders/ssao.vert.glsl.spv")};
			gpu::VKShader::Bytecode ps_bytecode{io::filesystem::readBinary(p_binary_dir / "shaders/ssao.pixel.glsl.spv")};
			TST_ASSERT_MSG(!vs_bytecode.empty() && !ps_bytecode.empty(), "Failed to read shader file. Did you add it to the CMake compilation");
			std::initializer_list<gpu::VKShader::Bytecode> bytecode = {vs_bytecode, ps_bytecode};
			const auto ssao_shader{make_reference<gpu::VKShader>(m_renderCtx->getLogicalDevice(), shader_stages, bytecode, "SSAO_Graphics")};
			m_shaderLibrary.add("SSAO_Graphics", ssao_shader);
		}
		{
			gpu::VKShader::Bytecode cs_bytecode{io::filesystem::readBinary(p_binary_dir / "shaders/diffuse_irradiance_convolution.comp.glsl.spv")};
			TST_ASSERT_MSG(!cs_bytecode.empty(), "Failed to read shader file. Did you add it to the CMake compilation");
			std::initializer_list<gpu::VKShader::Bytecode> bytecode = {cs_bytecode};
			auto stage = {vk::ShaderStageFlagBits::eCompute};
			const auto compute_test_shader{make_reference<gpu::VKShader>(m_renderCtx->getLogicalDevice(), stage, bytecode, "Diffuse_Irradiance_Convolution")};
			m_shaderLibrary.add("Diffuse_Irradiance_Convolution", compute_test_shader);
		}
		{
			gpu::VKShader::Bytecode cs_bytecode{io::filesystem::readBinary(p_binary_dir / "shaders/ssao_blur.comp.glsl.spv")};
			TST_ASSERT_MSG(!cs_bytecode.empty(), "Failed to read shader file. Did you add it to the CMake compilation");
			std::initializer_list<gpu::VKShader::Bytecode> bytecode = {cs_bytecode};
			auto stage = {vk::ShaderStageFlagBits::eCompute};
			const auto compute_test_shader{make_reference<gpu::VKShader>(m_renderCtx->getLogicalDevice(), stage, bytecode, "SSAO_Blur")};
			m_shaderLibrary.add("SSAO_Blur", compute_test_shader);
		}

		m_quadVertices.emplace_back(QuadVertex{{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}});
		m_quadVertices.emplace_back(QuadVertex{{1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}});
		m_quadVertices.emplace_back(QuadVertex{{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}});
		m_quadVertices.emplace_back(QuadVertex{{-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}});

		m_quadIndices = {0, 1, 3, 1, 2, 3};

		vk::DeviceSize vbo_size{m_quadVertices.size() * sizeof(QuadVertex)};
		m_quadVertexBuffer = make_reference<gpu::VKVertexBuffer>(m_renderCtx->getLogicalDevice(), m_quadVertices.data(), vbo_size);

		vk::DeviceSize ibo_size{m_quadIndices.size() * sizeof(uint32)};
		m_quadIndexBuffer = make_reference<gpu::VKIndexBuffer>(m_renderCtx->getLogicalDevice(), m_quadIndices.data(), ibo_size);

		gpu::TextureSpecInfo white_texture_spec_info{};
		white_texture_spec_info.size   = {1u};
		white_texture_spec_info.format = vk::Format::eR8G8B8A8Unorm;
		uint32 white_texture_data{0xFFFFFFFF};
		m_whiteTexture = make_reference<gpu::VKTexture2D>(m_renderCtx->getLogicalDevice(), white_texture_spec_info, &white_texture_data, sizeof(uint32));

		uint32 texture_3d_data[6]{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
		m_whiteTexture3D = make_reference<gpu::VKTexture3D>(m_renderCtx->getLogicalDevice(), white_texture_spec_info,
															toaster::Buffer{texture_3d_data, sizeof(uint32) * 6});

		Buffer image_data{};
		image_data.allocate(sizeof(uint32));
		image_data.writeType<uint32>(0xFFFFFFFF);

		ImageSpecInfo image_spec_info{};
		image_spec_info.size   = {1u, 1u};
		image_spec_info.format = vk::Format::eR8G8B8A8Unorm;
		m_whiteImage           = make_reference<Image>(*m_renderCtx, image_spec_info, image_data);

		image_data.release();
	}

	Globals::~Globals()
	{
	}

	auto Globals::shaderLibrary() const -> const ShaderLibrary &
	{
		return m_shaderLibrary;
	}

	auto Globals::dynamicShaderLibrary() const -> const DynamicShaderLibrary &
	{
		return m_dynamicShaderLibrary;
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

	auto Globals::whiteImage() const -> const ImageHandle &
	{
		return m_whiteImage;
	}
}
