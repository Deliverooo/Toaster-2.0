#pragma once

#include "image.hpp"
#include "shader_library.hpp"

#include "toast_gpu/vk/vk_index_buffer.hpp"
#include "toast_gpu/vk/vk_vertex_buffer.hpp"

#include "toast_gpu/vk/vk_texture.hpp"

namespace toaster::render
{
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
			Dx::XMFLOAT4   position;
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

		Globals(RenderContext &p_render_ctx, const io::filesystem::Path &p_binary_dir);

		~Globals();

		/*!
		 * @brief use ->get("") to get a shader
		 * The shaders included are "Depth-Pre", "Geometry", "Composite", "Skybox", "Quad", "Compute-Test"
		 * @return Returns the shader library
		 */
		auto shaderLibrary() const -> const ShaderLibrary &;
		auto dynamicShaderLibrary() const -> const DynamicShaderLibrary &;

		auto fullscreenQuadVertexBuffer() const -> const gpu::VertexBufferHandle &;
		auto fullscreenQuadIndexBuffer() const -> const gpu::IndexBufferHandle &;

		auto fullscreenQuadVertices() const -> const std::vector<FullscreenQuadVertex> &;
		auto fullscreenQuadIndices() const -> const std::vector<uint32> &;

		auto whiteTexture() const -> const gpu::Texture2DHandle &;
		auto whiteTexture3D() const -> const gpu::Texture3DHandle &;
		auto whiteImage() const -> const ImageHandle &;

	private:
		io::filesystem::Path m_binaryDir;

		ShaderLibrary        m_shaderLibrary;
		DynamicShaderLibrary m_dynamicShaderLibrary;

		RefPtr<gpu::VKVertexBuffer> m_quadVertexBuffer{nullptr};
		RefPtr<gpu::VKIndexBuffer>  m_quadIndexBuffer{nullptr};

		std::vector<FullscreenQuadVertex> m_quadVertices;
		std::vector<uint32>               m_quadIndices;

		gpu::Texture2DHandle m_whiteTexture{nullptr};
		gpu::Texture3DHandle m_whiteTexture3D{nullptr};
		ImageHandle          m_whiteImage{nullptr};
	};
}
