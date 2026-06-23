#pragma once

#include <unordered_set>

#include "image.hpp"
#include "shader_library.hpp"

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
			tsm::float3 position;
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

			char _padd[2];
		};

		Globals(RenderContext &p_render_ctx, const GlobalsSpecInfo &p_spec_info);

		auto getShader(const String &p_name) const -> const gpu::DynamicShaderHandle &;
		auto addShader(const String &p_name, const gpu::DynamicShaderHandle &p_shader) -> void;

		auto fullscreenQuadVertexBuffer() const -> const gpu::VertexBufferHandle &;
		auto fullscreenQuadIndexBuffer() const -> const gpu::IndexBufferHandle &;

		auto fullscreenQuadVertices() const -> const std::vector<FullscreenQuadVertex> &;
		auto fullscreenQuadIndices() const -> const std::vector<uint32> &;

		auto whiteTexture() const -> const gpu::Texture2DHandle &;
		auto whiteTexture3D() const -> const gpu::Texture3DHandle &;
		auto whiteImage() const -> const ImageHandle &;

		auto debugImage() const -> const ImageHandle &;

	private:
		GlobalsSpecInfo m_specInfo{};

		std::unordered_map<String, gpu::DynamicShaderHandle> m_shaders;

		// DynamicShaderLibrary m_dynamicShaderLibrary;

		RefPtr<gpu::VKVertexBuffer> m_quadVertexBuffer{nullptr};
		RefPtr<gpu::VKIndexBuffer>  m_quadIndexBuffer{nullptr};

		std::vector<FullscreenQuadVertex> m_quadVertices;
		std::vector<uint32>               m_quadIndices;

		gpu::Texture2DHandle m_whiteTexture{nullptr};
		gpu::Texture3DHandle m_whiteTexture3D{nullptr};
		ImageHandle          m_whiteImage{nullptr};

		ImageHandle m_debugImage{nullptr};
	};
}
