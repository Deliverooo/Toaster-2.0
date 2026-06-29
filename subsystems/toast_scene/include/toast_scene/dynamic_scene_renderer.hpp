#pragma once

#include "scene.hpp"
#include "toast_render/dynamic_mesh.hpp"
#include "toast_render/skybox_pass.hpp"

namespace toaster
{
	class TST_SCENE_API DynamicSceneRenderer
	{
	public:
		DynamicSceneRenderer(Scene *p_scene, tsm::uint2 p_viewport_size);
		~DynamicSceneRenderer();

		auto             onRender() -> void;
		auto XM_CALLCONV onRender(Dx::FXMVECTOR p_camera_position, Dx::FXMMATRIX p_view_matrix, Dx::CXMMATRIX p_projection_matrix) -> void;

		// Passing the position avoids unnecessary calculations
		auto XM_CALLCONV begin(Dx::FXMVECTOR p_camera_position, Dx::FXMMATRIX p_view_matrix, Dx::CXMMATRIX p_projection_matrix) -> void;
		auto             end() -> void;
		auto XM_CALLCONV renderMesh(const render::DynamicMeshHandle &p_mesh, Dx::FXMMATRIX p_transform) -> void;

		auto getViewportSize() const -> tsm::uint2 { return m_viewportSize; }

		auto getColourImage() const -> const render::ImageHandle & { return m_colourRenderTarget; }
		auto getDepthImage() const -> const render::ImageHandle & { return m_depthRenderTarget; }

		auto onResize(tsm::uint2 p_size) -> void;

		auto reloadEnvironmentMaps(const render::ImageHandle &p_skybox, const render::ImageHandle &p_diffuse_irradiance) -> void;

	private:
		auto _performMeshTransformPrePass() -> void;
		auto _renderDepthPrePass(gpu::VKCommandBuffer *p_cmd) -> void;
		auto _renderSkyboxPass(gpu::VKCommandBuffer *p_cmd) -> void;
		auto _renderGeometryPass(gpu::VKCommandBuffer *p_cmd) -> void;

		NonOwningPtr<Scene>                 m_scene{nullptr};
		NonOwningPtr<render::RenderContext> m_renderCtx{nullptr};

		tsm::uint2 m_viewportSize{0u};

		render::UniformBufferPFFUnique m_cameraUBOs{nullptr};

		struct SceneDataUB
		{
			Dx::XMFLOAT4 cameraPos{0.0f, 0.0f, 0.0f, 1.0f};
		};

		render::UniformBufferPFFUnique m_sceneDataUBOs{nullptr};

		render::ImageHandle m_colourRenderTarget{nullptr};
		render::ImageHandle m_depthRenderTarget{nullptr};

		render::GraphicsStateUnique m_depthPreGraphicsState{nullptr};
		TST_PUSH_CONSTANT_BLOCK(DepthPreConstants)
		{
			Dx::XMFLOAT4X4 meshTransform;

			uintptr vertexBuffer;

			uintptr cameraPtr;
		};

		render::ImageHandle           m_diffuseIrradianceMap{nullptr};
		UniquePtr<render::SkyboxPass> m_skyboxPass{nullptr};

		render::GraphicsStateUnique m_geometryGraphicsState{nullptr};
		TST_PUSH_CONSTANT_BLOCK(MeshDrawConstants)
		{
			Dx::XMFLOAT4X4 meshTransform;

			uintptr vertexBuffer;

			uintptr material;

			uintptr cameraPtr;
			uintptr sceneDataPtr;

			uint32 samplerIndex;
			uint32 diffuseIrradianceMapIndex;
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
