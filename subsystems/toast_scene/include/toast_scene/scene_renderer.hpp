#pragma once

#include "scene.hpp"
#include "toast_gpu/vk/vk_storage_image.hpp"
#include "toast_gpu/vk/vk_uniform_buffer.hpp"
#include "toast_render/compute_pass.hpp"
#include "toast_render/mesh.hpp"

namespace toaster
{
	struct TST_SCENE_API SceneRendererSpecInfo
	{
		tsm::uint2 viewportSize{0u};
		tsm::uint2 viewportOffset{0u};

		gpu::ShaderHandle overrideGeometryShader{nullptr}; // Pretty damn useful

		bool backfaceCulling{true};
	};

	class TST_SCENE_API SceneRenderer
	{
	public:
		SceneRenderer(Scene *p_scene, const SceneRendererSpecInfo &p_spec_info);
		~SceneRenderer();

		auto             onRender() -> void;
		auto XM_CALLCONV onRender(Dx::FXMVECTOR p_camera_position, Dx::FXMMATRIX p_view_matrix, Dx::CXMMATRIX p_projection_matrix) -> void;
		auto             onRender(gpu::VKCommandBuffer *p_cmd) -> void;
		auto XM_CALLCONV onRender(gpu::VKCommandBuffer *p_cmd, Dx::FXMVECTOR p_camera_position, Dx::FXMMATRIX p_view, Dx::CXMMATRIX p_projection) -> void;

		// Passing the position avoids unnecessary calculations
		auto XM_CALLCONV begin(Dx::FXMVECTOR p_camera_position, Dx::FXMMATRIX p_view_matrix, Dx::CXMMATRIX p_projection_matrix) -> void;
		auto             end(gpu::VKCommandBuffer *p_cmd) -> void;
		auto XM_CALLCONV renderMesh(const render::MeshHandle &p_mesh, Dx::FXMMATRIX p_transform) -> void;

		auto getSpecInfo() const -> const SceneRendererSpecInfo & { return m_specInfo; }

		auto getMSAAColourImage() -> gpu::RawImageHandle & { return m_MSAAcolourImage; }
		auto getMSAADepthImage() -> gpu::RawImageHandle & { return m_MSAADepthImage; }
		auto getMSAAGeometryNormalsImage() -> gpu::RawImageHandle & { return m_MSAAGeometryNormalsImage; }
		auto getMSAAGeometryPositionsImage() -> gpu::RawImageHandle & { return m_MSAAGeometryPositionsImage; }

		auto getColourTexture() const -> const gpu::Texture2DHandle & { return m_colourTexture; }
		auto getDepthTexture() const -> const gpu::Texture2DHandle & { return m_depthTexture; }
		auto getGeometryNormalsTexture() const -> const gpu::Texture2DHandle & { return m_geometryNormalsTexture; }
		auto getGeometryPositionsTexture() const -> const gpu::Texture2DHandle & { return m_geometryPositionsTexture; }

		auto getSSAONoiseTexture() const -> const gpu::Texture2DHandle & { return m_SSAONoiseTexture; }
		auto getSSAOTexture() const -> const gpu::Texture2DHandle & { return m_SSAOTexture; }
		auto getSSAOBlurredImage() const -> const gpu::StorageImageHandle & { return m_SSAOBlurredImage; }

		auto getRenderer2D() -> const RefPtr<render::Renderer2D> & { return m_renderer2D; }

		auto onResize(tsm::uint2 p_size) -> void;

		auto reloadEnvironmentMaps(const gpu::Texture3DHandle &p_skybox, const gpu::Texture3DHandle &p_diffuse_irradiance) -> void;

	private:
		auto _renderDepthPrePass(gpu::VKCommandBuffer *p_cmd) -> void;
		auto _renderAOPass(gpu::VKCommandBuffer *p_cmd) -> void;
		auto _renderSkyboxPass(gpu::VKCommandBuffer *p_cmd) -> void;
		auto _renderGeometryPass(gpu::VKCommandBuffer *p_cmd) -> void;

		static auto _generateSSAONoise(uint32 p_texture_size) -> std::vector<tsm::float4>;

		NonOwningPtr<Scene> m_scene{nullptr};

		NonOwningPtr<render::RenderContext> m_renderCtx{nullptr};

		SceneRendererSpecInfo m_specInfo{};

		RefPtr<render::Renderer2D> m_renderer2D{nullptr};

		#pragma region depth-pre

		// render::GraphicsStateUnique m_depthPreGraphicsState{nullptr};

		gpu::PipelineHandle      m_depthPrePipeline{nullptr};
		render::RenderPassHandle m_depthPrePass{nullptr};

		gpu::RawImageHandle  m_MSAADepthImage{nullptr};
		gpu::Texture2DHandle m_depthTexture{nullptr};

		gpu::RawImageHandle  m_MSAAGeometryNormalsImage{nullptr};
		gpu::Texture2DHandle m_geometryNormalsTexture{nullptr};

		gpu::RawImageHandle  m_MSAAGeometryPositionsImage{nullptr};
		gpu::Texture2DHandle m_geometryPositionsTexture{nullptr};
		#pragma endregion

		#pragma region ambient occlusion

		gpu::PipelineHandle      m_SSAOPipeline{nullptr};
		render::RenderPassHandle m_SSAOPass{nullptr};

		render::MaterialHandle m_SSAOFrameDataMaterial{nullptr};

		struct SSAOKernel
		{
			static constexpr uint32 c_SSAOSampleCount{64};

			Dx::XMFLOAT4 samples[c_SSAOSampleCount];

			SSAOKernel();
		};

		UniquePtr<SSAOKernel> m_SSAOKernel{nullptr};

		gpu::Texture2DHandle m_SSAONoiseTexture{nullptr};
		gpu::Texture2DHandle m_SSAOTexture{nullptr};

		gpu::ComputePipelineHandle m_SSAOBlurPipeline{nullptr};
		render::ComputePassHandle  m_SSAOBlurPass{nullptr};
		gpu::StorageImageHandle    m_SSAOBlurredImage{nullptr};

		#pragma endregion

		#pragma region skybox
		gpu::PipelineHandle      m_skyboxPipeline{nullptr};
		render::RenderPassHandle m_skyboxPass{nullptr};
		render::MaterialHandle   m_skyboxMaterial{nullptr};

		#pragma endregion

		#pragma region geometry
		gpu::PipelineHandle      m_geometryPipeline{nullptr};
		render::RenderPassHandle m_geometryPass{nullptr};
		#pragma endregion

		gpu::RawImageHandle  m_MSAAcolourImage{nullptr};
		gpu::Texture2DHandle m_colourTexture{nullptr};

		struct CameraUB
		{
			Dx::XMFLOAT4X4 view;
			Dx::XMFLOAT4X4 proj;
			Dx::XMFLOAT4X4 invProj;
		};

		gpu::UniformBufferPFFHandle m_cameraUBOs;
		gpu::UBOMappedDataPFF       m_mappedCameraUBOs;

		struct DirectionalLightUB
		{
			static constexpr uint32 c_maxDirectionalLights{4u};

			uint32           count{0u};
			tsm::float3      _padding{0.0f};
			DirectionalLight directionalLights[c_maxDirectionalLights]{};
		};

		gpu::UniformBufferPFFHandle m_directionalLightUBOs;
		gpu::UBOMappedDataPFF       m_mappedDirectionalLightUBOs;

		struct PointLightUB
		{
			static constexpr uint32 c_maxPointLights{128u};

			uint32      count{0u};
			tsm::float3 _padding{0.0f};
			PointLight  pointLights[c_maxPointLights]{};
		};

		gpu::UniformBufferPFFHandle m_pointLightUBOs;
		gpu::UBOMappedDataPFF       m_mappedPointLightUBOs;

		struct SceneDataUB
		{
			Dx::XMFLOAT3 cameraPos{0.0f, 0.0f, 0.0f};
			char         _padd[4];
		};

		gpu::UniformBufferPFFHandle m_sceneDataUBOs;
		gpu::UBOMappedDataPFF       m_mappedSceneDataUBOs;

		struct DrawCommand
		{
			render::MeshHandle mesh{nullptr};
			Dx::XMFLOAT4X4     transform;
		};

		std::vector<DrawCommand> m_meshDrawCommands;
	};
}
