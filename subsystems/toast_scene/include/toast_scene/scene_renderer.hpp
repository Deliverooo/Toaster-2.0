#pragma once

#include "scene.hpp"
#include "toast_render/dynamic_mesh.hpp"
#include "toast_render/uniform_buffer.hpp"

namespace toaster::scene
{
	class TST_SCENE_API SceneRenderer
	{
	public:
		SceneRenderer(Scene &p_scene, tsm::uint2 p_initial_viewport_size);
		~SceneRenderer();

		auto onRender() -> void;

		// Passing the position avoids unnecessary calculations
		auto XM_CALLCONV onRender(Dx::FXMVECTOR p_camera_position, Dx::FXMMATRIX p_view_matrix, Dx::CXMMATRIX p_projection_matrix) -> void;

		auto XM_CALLCONV submitMesh(const render::DynamicMeshHandle &p_mesh, Dx::FXMMATRIX p_transform) -> void;

		auto getViewportSize() const -> tsm::uint2 { return m_viewportSize; }

		auto getColourImage() const -> const render::ImageHandle & { return m_colourRenderTarget; }
		auto getDepthImage() const -> const render::ImageHandle & { return m_depthRenderTarget; }

		auto onResize(tsm::uint2 p_size) -> void;

	private:
		auto XM_CALLCONV _begin(Dx::FXMVECTOR p_camera_position, Dx::FXMMATRIX p_view_matrix, Dx::CXMMATRIX p_projection_matrix) -> void;
		auto             _end() -> void;

		auto _buildDrawCommands() -> void;
		auto _setRequiredRenderState(gpu::CommandBuffer &p_cmd) -> void;
		auto _renderDepthPrePass(gpu::CommandBuffer &p_cmd) -> void;
		auto _renderSkyboxPass(gpu::CommandBuffer &p_cmd) -> void;
		auto _renderGeometryPass(gpu::CommandBuffer &p_cmd) -> void;

		NonOwningPtr<const Scene>           m_scene{nullptr};
		NonOwningPtr<render::RenderContext> m_renderCtx{nullptr};

		tsm::uint2 m_viewportSize{0u};

		render::UniformBufferPFFUnique m_cameraUBOs{nullptr};

		struct SceneDataUB
		{
			Dx::XMFLOAT4 cameraPos{0.0f, 0.0f, 0.0f, 1.0f};
		};

		render::UniformBufferPFFUnique m_sceneDataUBOs{nullptr};

		struct PointLightSSBO
		{
			static constexpr uint32 maxPointLights{128u};

			uint32  count{0u};
			float32 _padd[3];

			PointLight pointLights[maxPointLights];
		};

		render::StorageBufferPFFUnique m_pointLightSSBOs{nullptr};

		render::ImageHandle m_colourRenderTarget{nullptr};
		render::ImageHandle m_depthRenderTarget{nullptr};

		TST_PUSH_CONSTANT_BLOCK(DepthPreConstants)
		{
			Dx::XMFLOAT4X4 meshTransform;

			uintptr vertexBuffer;
			// uintptr indexBuffer;
			uintptr cameraPtr;

			// uint32 indexOffset;
			// uint32 vertexOffset;
		};

		TST_PUSH_CONSTANT_BLOCK(SkyboxConstants)
		{
			uintptr vertexBufferBDA;
			uintptr cameraBDA;
			uint32  samplerAddressOffset;
			uint32  environmentMapAddressOffset;
		};

		TST_PUSH_CONSTANT_BLOCK(MeshDrawConstants)
		{
			Dx::XMFLOAT4X4 meshTransform;

			uintptr vertexBuffer;
			// uintptr indexBuffer;
			uintptr pointLightsPtr;

			// uint32 indexOffset;
			// uint32 vertexOffset;

			uint32 materialIndex;

			uint32 cameraIndex;
			uint32 sceneDataIndex;

			uint32 samplerIndex;
			uint32 diffuseIrradianceMapIndex;
			uint32 specularIrradianceMapIndex;

			uint32 BRDFLUTSamplerIndex;
			uint32 BRDFLUT;
		};

		struct MeshDrawCommand
		{
			Dx::XMFLOAT4X4            transform;
			render::DynamicMeshHandle mesh{nullptr};
		};

		std::vector<MeshDrawCommand> m_meshDrawCommands;

		struct SubmeshDrawCommand
		{
			Dx::XMFLOAT4X4            transform;
			render::DynamicMeshHandle mesh{nullptr};

			int32  vertexOffset{0};
			uint32 vertexCount{0u};
			uint32 indexOffset{0u};
			uint32 indexCount{0u};

			uint32 materialIndex{0u};
		};

		std::vector<SubmeshDrawCommand> m_submeshDrawCommands;
	};
}
