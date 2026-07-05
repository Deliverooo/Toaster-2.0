#pragma once

#include <unordered_set>

#include "image.hpp"
#include "shader_library.hpp"
#include "shader_reflection.hpp"
#include "storage_buffer.hpp"

#include "toast_gpu/vk/vk_index_buffer.hpp"
#include "toast_gpu/vk/vk_vertex_buffer.hpp"

#include "toast_gpu/vk/vk_texture.hpp"

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

		auto getShader(const String &p_name) const -> const gpu::DynamicShaderHandle &;
		auto addShader(const String &p_name, const gpu::DynamicShaderHandle &p_shader) -> void;

		struct ShaderReflectionData
		{
			reflection::ReflectionData  reflectionData;
			reflection::ReflectedStruct materialStruct;
		};

		auto reflectShader(const String &p_name) -> const ShaderReflectionData &;
		auto getShaderReflectionData(const String &p_name) const -> const ShaderReflectionData &;

		auto fullscreenQuadVertexBuffer() const -> const VertexBuffer &;
		auto fullscreenQuadIndexBuffer() const -> const gpu::Buffer &;

		auto fullscreenQuadVertices() const -> const std::vector<FullscreenQuadVertex> &;
		auto fullscreenQuadIndices() const -> const std::vector<uint8> &;

		auto whiteTexture() const -> const gpu::Texture2DHandle &;
		auto whiteTexture3D() const -> const gpu::Texture3DHandle &;
		auto whiteImage() const -> const ImageHandle &;

		auto debugImage() const -> const ImageHandle &;
		auto BRDFLUT() const -> const ImageHandle &;

	private:
		GlobalsSpecInfo m_specInfo{};

		std::unordered_map<String, gpu::DynamicShaderHandle> m_shaders;
		std::unordered_map<String, ShaderReflectionData>     m_shaderReflectionData;

		VertexBufferUnique m_quadVertexBuffer{nullptr};
		gpu::BufferUnique  m_quadIndexBuffer{nullptr};

		std::vector<FullscreenQuadVertex> m_quadVertices;
		std::vector<uint8>                m_quadIndices;

		gpu::Texture2DHandle m_whiteTexture{nullptr};
		gpu::Texture3DHandle m_whiteTexture3D{nullptr};
		ImageHandle          m_whiteImage{nullptr};

		ImageHandle m_debugImage{nullptr};

		ImageHandle m_BRDFLUT{nullptr};
	};
}
