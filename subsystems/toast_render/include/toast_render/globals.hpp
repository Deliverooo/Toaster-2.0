#pragma once

#include "shader_library.hpp"
#include "shader_reflection.hpp"
#include "storage_buffer.hpp"

namespace toaster::render
{
	struct TST_RENDER_API GlobalsSpecInfo
	{
		io::filesystem::Path shaderBinaryDir; // From the sdk, or local to the project...
	};

	class TST_RENDER_API Globals final
	{
		TST_RENDER_OBJECT
	public:
		struct FullscreenQuadVertex
		{
			tsm::float2 position;
			tsm::float2 texCoord;
		};

		// Useful
		struct CameraUB
		{
			Dx::XMFLOAT4X4 viewMatrix;
			Dx::XMFLOAT4X4 projectionMatrix;
			Dx::XMFLOAT4X4 inverseProjectionMatrix;
		};

		// Use if you only need the view and projection matrix...
		struct ViewProjCameraUB
		{
			Dx::XMFLOAT4X4 viewMatrix;
			Dx::XMFLOAT4X4 projectionMatrix;
		};

		TST_PUSH_CONSTANT_BLOCK(EquirectangularToCubeMapConstants)
		{
			uint32 equirectangularMapId;
			uint32 cubeMapId;
			uint32 samplerId;

			char _padd[2];
		};

		TST_PUSH_CONSTANT_BLOCK(DiffuseIrradianceConvolutionConstants)
		{
			uint32 environmentMapId;
			uint32 diffuseIrradianceMapId;
			uint32 samplerId;

			char _padd[4];
		};

		TST_PUSH_CONSTANT_BLOCK(SpecularIrradianceConvolutionConstants)
		{
			uint32 environmentMapId;
			uint32 specularIrradianceMapId;
			uint32 samplerId;

			uint32  numSamples;
			float32 roughness;

			char _padd[12];
		};

		Globals(RenderContext &p_render_ctx, const GlobalsSpecInfo &p_spec_info);

		auto getShader(const String &p_name) const -> const gpu::ShaderHandle &;
		auto addShader(const String &p_name, const gpu::ShaderHandle &p_shader) -> void;

		struct ShaderReflectionData
		{
			reflection::ReflectionData  reflectionData;
			reflection::ReflectedStruct materialStruct;
			reflection::ReflectedStruct constantBufferStruct;
		};

		auto reflectShader(const String &p_name) -> const ShaderReflectionData &;
		auto getShaderReflectionData(const String &p_name) const -> const ShaderReflectionData &;

		auto fullscreenQuadVertexBuffer() const -> const VertexBuffer &;
		auto fullscreenQuadIndexBuffer() const -> const gpu::Buffer &;

		auto fullscreenQuadVertices() const -> const std::vector<FullscreenQuadVertex> &;
		auto fullscreenQuadIndices() const -> const std::vector<uint8> &;

		auto whiteImage() const -> const gpu::ImageHandle &;

		auto debugImage() const -> const gpu::ImageHandle &;
		auto BRDFLUT() const -> const gpu::ImageHandle &;

	private:
		GlobalsSpecInfo m_specInfo{};

		std::unordered_map<String, gpu::ShaderHandle>    m_shaders;
		std::unordered_map<String, ShaderReflectionData> m_shaderReflectionData;

		VertexBufferUnique m_quadVertexBuffer{nullptr};
		gpu::BufferUnique  m_quadIndexBuffer{nullptr};

		std::vector<FullscreenQuadVertex> m_quadVertices;
		std::vector<uint8>                m_quadIndices;

		gpu::ImageHandle m_whiteImage{nullptr};

		gpu::ImageHandle m_debugImage{nullptr};

		gpu::ImageHandle m_BRDFLUT{nullptr};
	};
}
