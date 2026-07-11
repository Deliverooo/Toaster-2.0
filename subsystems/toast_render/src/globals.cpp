#include "toast_render/globals.hpp"

#include "toast_gpu/vk/vk_logical_device.hpp"
#include "toast_lib/io/filesystem.hpp"
#include "toast_render/constant_buffer.hpp"
#include "toast_render/dynamic_material.hpp"
#include "toast_render/render_context.hpp"

namespace toaster::render
{
	Globals::Globals(RenderContext &p_render_ctx, const GlobalsSpecInfo &p_spec_info) : m_renderCtx(&p_render_ctx), m_specInfo(p_spec_info)
	{
		#pragma region shaders
		DEBUG_LOG_TRACE("Shader binary dir: {}", m_specInfo.shaderBinaryDir);
		TST_ASSERT(io::filesystem::exists(m_specInfo.shaderBinaryDir));

		// Fullscreen quad shaders
		addShader("Fullscreen_Quad_MS",
				  m_renderCtx->createShaderFromSpirV(m_specInfo.shaderBinaryDir / "fullscreen_quad.mesh.glsl.spv", EShaderStage::eMesh, EShaderStage::ePixel));
		addShader("Fullscreen_Quad_VS",
				  m_renderCtx->createShaderFromSpirV(m_specInfo.shaderBinaryDir / "fullscreen_quad.vert.glsl.spv", EShaderStage::eVertex, EShaderStage::ePixel));
		addShader("Fullscreen_Quad_PS", m_renderCtx->createShaderFromSpirV(m_specInfo.shaderBinaryDir / "fullscreen_quad.pixel.glsl.spv", EShaderStage::ePixel));

		// Dynamic mesh/meshlet shaders
		addShader("Dynamic_Mesh_VS",
				  m_renderCtx->createShaderFromSpirV(m_specInfo.shaderBinaryDir / "dynamic_mesh.vert.glsl.spv", EShaderStage::eVertex, EShaderStage::ePixel));
		addShader("Dynamic_Mesh_TS",
				  m_renderCtx->createShaderFromSpirV(m_specInfo.shaderBinaryDir / "dynamic_mesh.task.glsl.spv", EShaderStage::eTask, EShaderStage::eMesh));
		addShader("Dynamic_Mesh_MS",
				  m_renderCtx->createShaderFromSpirV(m_specInfo.shaderBinaryDir / "dynamic_mesh.mesh.glsl.spv", EShaderStage::eMesh, EShaderStage::ePixel));
		addShader("Dynamic_Mesh_PS", m_renderCtx->createShaderFromSpirV(m_specInfo.shaderBinaryDir / "dynamic_mesh.pixel.glsl.spv", EShaderStage::ePixel));

		addShader("Default_Unlit_VS",
				  m_renderCtx->createShaderFromSpirV(m_specInfo.shaderBinaryDir / "default_unlit.vert.glsl.spv", EShaderStage::eVertex, EShaderStage::ePixel));
		addShader("Default_Unlit_PS", m_renderCtx->createShaderFromSpirV(m_specInfo.shaderBinaryDir / "default_unlit.pixel.glsl.spv", EShaderStage::ePixel));

		// Quad shaders
		addShader("Quad_VS", m_renderCtx->createShaderFromSpirV(m_specInfo.shaderBinaryDir / "quad_dynamic.vert.hlsl.spv", EShaderStage::eVertex, EShaderStage::ePixel));
		addShader("Quad_PS", m_renderCtx->createShaderFromSpirV(m_specInfo.shaderBinaryDir / "quad_dynamic.pixel.glsl.spv", EShaderStage::ePixel));

		// Equirectangular to cube map shader
		addShader("Equirectangular_To_Cube_Map",
				  m_renderCtx->createShaderFromSpirV(m_specInfo.shaderBinaryDir / "equirectangular_to_cube_map.comp.glsl.spv", EShaderStage::eCompute));

		// Diffuse irradiance convolution shader
		addShader("Diffuse_Irradiance_Convolution",
				  m_renderCtx->createShaderFromSpirV(m_specInfo.shaderBinaryDir / "diffuse_irradiance_convolution.comp.glsl.spv", EShaderStage::eCompute));

		// Specular irradiance convolution shader
		addShader("Specular_Irradiance_Convolution",
				  m_renderCtx->createShaderFromSpirV(m_specInfo.shaderBinaryDir / "specular_irradiance_convolution.comp.glsl.spv", EShaderStage::eCompute));

		// Depth pre shaders
		addShader("Depth_Pre_VS", m_renderCtx->createShaderFromSpirV(m_specInfo.shaderBinaryDir / "depth_pre.vert.glsl.spv", EShaderStage::eVertex,
																	 EShaderStage::ePixel));
		addShader("Depth_Pre_PS", m_renderCtx->createShaderFromSpirV(m_specInfo.shaderBinaryDir / "depth_pre.pixel.glsl.spv", EShaderStage::ePixel));

		// Skybox shaders
		addShader("Skybox_VS", m_renderCtx->createShaderFromSpirV(m_specInfo.shaderBinaryDir / "skybox.vert.glsl.spv", EShaderStage::eVertex, EShaderStage::ePixel));
		addShader("Skybox_PS", m_renderCtx->createShaderFromSpirV(m_specInfo.shaderBinaryDir / "skybox.pixel.glsl.spv", EShaderStage::ePixel));

		// Mesh geometry shaders
		addShader("Mesh_Geo_VS", m_renderCtx->createShaderFromSpirV(m_specInfo.shaderBinaryDir / "mesh_geo.vert.hlsl.spv", EShaderStage::eVertex, EShaderStage::ePixel));
		addShader("Mesh_Geo_PS", m_renderCtx->createShaderFromSpirV(m_specInfo.shaderBinaryDir / "mesh_geo.pixel.glsl.spv", EShaderStage::ePixel));

		addShader("Test_Shader_MS",
				  m_renderCtx->createShaderFromSpirV(m_specInfo.shaderBinaryDir / "test_shader.mesh.glsl.spv", EShaderStage::eMesh, EShaderStage::ePixel));
		addShader("Test_Shader_PS", m_renderCtx->createShaderFromSpirV(m_specInfo.shaderBinaryDir / "test_shader.pixel.glsl.spv", EShaderStage::ePixel));
		#pragma endregion

		m_quadVertices.emplace_back(FullscreenQuadVertex{{1.0f, 1.0f}, {1.0f, 1.0f}});
		m_quadVertices.emplace_back(FullscreenQuadVertex{{1.0f, -1.0f}, {1.0f, 0.0f}});
		m_quadVertices.emplace_back(FullscreenQuadVertex{{-1.0f, -1.0f}, {0.0f, 0.0f}});
		m_quadVertices.emplace_back(FullscreenQuadVertex{{-1.0f, 1.0f}, {0.0f, 1.0f}});

		m_quadIndices = {0, 1, 3, 1, 2, 3};

		uint64              index_buffer_size{m_quadIndices.size() * sizeof(uint8)};
		gpu::BufferSpecInfo staging_buffer_spec{};
		staging_buffer_spec.deviceLocal = false;
		staging_buffer_spec.usageFlags  = vk::BufferUsageFlagBits2::eTransferSrc;
		gpu::Buffer staging_buffer{*m_renderCtx->getGPUContext(), index_buffer_size, staging_buffer_spec};
		staging_buffer.copyData(m_quadIndices.data(), index_buffer_size);

		gpu::BufferSpecInfo index_buffer_spec_info;
		index_buffer_spec_info.deviceLocal = true;
		index_buffer_spec_info.usageFlags  = vk::BufferUsageFlagBits2::eTransferDst | vk::BufferUsageFlagBits2::eIndexBuffer;
		m_quadIndexBuffer                  = m_renderCtx->createGPUUnique<gpu::Buffer>(index_buffer_size, index_buffer_spec_info);

		m_quadIndexBuffer->copyFromBuffer(staging_buffer);

		m_quadVertexBuffer = m_renderCtx->createUnique<StorageBuffer>(m_quadVertices, true);

		{
			Buffer image_data{};
			image_data.allocate(sizeof(uint32));
			image_data.writeType<uint32>(0xFFFFFFFF);

			ImageSpecInfo image_spec_info{};
			image_spec_info.size   = {1u, 1u};
			image_spec_info.format = vk::Format::eR8G8B8A8Unorm;
			m_whiteImage           = make_reference<Image>(*m_renderCtx, image_spec_info, image_data);

			image_data.release();
		}

		{
			Buffer image_data{};
			image_data.allocate(sizeof(uint32) * 4u);
			image_data.writeType<uint32>(0xFFFF00FF);
			image_data.writeType<uint32>(0x00000000, sizeof(uint32));
			image_data.writeType<uint32>(0x00000000, sizeof(uint32) * 2u);
			image_data.writeType<uint32>(0xFFFF00FF, sizeof(uint32) * 3u);

			ImageSpecInfo image_spec_info{};
			image_spec_info.size   = {2u, 2u};
			image_spec_info.format = vk::Format::eR8G8B8A8Unorm;
			m_debugImage           = make_reference<Image>(*m_renderCtx, image_spec_info, image_data);

			image_data.release();
		}

		m_BRDFLUT = m_renderCtx->createImageRef(m_specInfo.shaderBinaryDir / "../../resources/textures/BRDF_LUT.png");
	}

	auto Globals::getShader(const String &p_name) const -> const gpu::ShaderHandle &
	{
		return m_shaders.at(p_name);
	}

	auto Globals::addShader(const String &p_name, const gpu::ShaderHandle &p_shader) -> void
	{
		TST_PERMA_ASSERT(!m_shaders.contains(p_name));
		m_shaders[p_name] = p_shader;
	}

	auto Globals::reflectShader(const String &p_name) -> const ShaderReflectionData &
	{
		auto shader{m_shaders.at(p_name)};

		auto &data{m_shaderReflectionData[p_name]};
		data.reflectionData = reflection::reflectShader(*shader);

		if (const auto mat_struct{findMaterialDeclaration(data.reflectionData)})
			data.materialStruct = *mat_struct;

		if (const auto cb_struct{findConstantBufferDeclaration(data.reflectionData)})
			data.constantBufferStruct = *cb_struct;

		return m_shaderReflectionData.at(p_name);
	}

	auto Globals::getShaderReflectionData(const String &p_name) const -> const ShaderReflectionData &
	{
		return m_shaderReflectionData.at(p_name);
	}

	auto Globals::fullscreenQuadVertexBuffer() const -> const VertexBuffer &
	{
		return *m_quadVertexBuffer;
	}

	auto Globals::fullscreenQuadIndexBuffer() const -> const gpu::Buffer &
	{
		return *m_quadIndexBuffer;
	}

	auto Globals::fullscreenQuadVertices() const -> const std::vector<FullscreenQuadVertex> &
	{
		return m_quadVertices;
	}

	auto Globals::fullscreenQuadIndices() const -> const std::vector<uint8> &
	{
		return m_quadIndices;
	}

	auto Globals::whiteImage() const -> const ImageHandle &
	{
		return m_whiteImage;
	}

	auto Globals::debugImage() const -> const ImageHandle &
	{
		return m_debugImage;
	}

	auto Globals::BRDFLUT() const -> const ImageHandle &
	{
		return m_BRDFLUT;
	}
}
