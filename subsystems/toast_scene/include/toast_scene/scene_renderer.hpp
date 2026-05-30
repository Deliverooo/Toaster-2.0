#pragma once

#include "scene.hpp"
#include "toast_render/mesh.hpp"

namespace toaster
{
	struct TST_SCENE_API SceneRendererSpecInfo
	{
		io::filesystem::Path resourceDirectory{};

		tsm::uint2 viewportSize{0u};
		tsm::uint2 viewportOffset{0u};
	};

	class TST_SCENE_API SceneRenderer
	{
	public:
		SceneRenderer(Scene *p_scene, const SceneRendererSpecInfo &p_spec_info);
		~SceneRenderer();

		auto onRender() -> void;
		auto onRender(const tsm::float4x4 &p_view, const tsm::float4x4 &p_projection) -> void;
		auto onRender(gpu::VKCommandBuffer *p_cmd) -> void;
		auto onRender(gpu::VKCommandBuffer *p_cmd, const tsm::float4x4 &p_view, const tsm::float4x4 &p_projection) -> void;

		auto begin(const tsm::float4x4 &p_view_matrix, const tsm::float4x4 &p_projection_matrix) -> void;
		auto end(gpu::VKCommandBuffer *p_cmd) -> void;
		auto renderMesh(const render::MeshHandle &p_mesh, const tsm::float4x4 &p_transform) -> void;

		auto getSpecInfo() const -> const SceneRendererSpecInfo &;

		auto getMSAAOutputColourImage() -> gpu::RawImageHandle &;
		auto getMSAAOutputDepthImage() -> gpu::RawImageHandle &;
		auto getMSAAOutputGeometryNormalsImage() -> gpu::RawImageHandle &;
		auto getMSAAOutputGeometryPositionsImage() -> gpu::RawImageHandle &;

		auto getResolveOutputColourTexture() const -> const gpu::Texture2DHandle &;
		auto getResolveOutputDepthTexture() const -> const gpu::Texture2DHandle &;
		auto getResolveOutputGeometryNormalsTexture() const -> const gpu::Texture2DHandle &;
		auto getResolveOutputGeometryPositionsTexture() const -> const gpu::Texture2DHandle &;

		auto getSSAONoiseTexture() const -> const gpu::Texture2DHandle &;
		auto getOutputAOTexture() const -> const gpu::Texture2DHandle &;
		auto getOutputAOBlurredImage() const -> const gpu::StorageImageHandle & { return m_aoBlurredOutputImage; }

		auto getOutputComputeImage() const -> const gpu::StorageImageHandle &;
		auto getRenderer2D() -> RefPtr<render::Renderer2D>;

		auto onResize(tsm::uint2 p_size) -> void;

		auto reloadEnvironmentMaps(const gpu::Texture3DHandle &p_skybox, const gpu::Texture3DHandle &p_diffuse_irradiance) -> void;

	private:
		auto _renderDepthPrePass(gpu::VKCommandBuffer *p_cmd) -> void;
		auto _renderAOPass(gpu::VKCommandBuffer *p_cmd) -> void;
		auto _renderLightCullingPass(gpu::VKCommandBuffer *p_cmd) -> void;
		auto _renderSkyboxPass(gpu::VKCommandBuffer *p_cmd) -> void;
		auto _renderGeometryPass(gpu::VKCommandBuffer *p_cmd) -> void;

		NonOwningPtr<Scene> m_scene{nullptr};

		NonOwningPtr<render::RenderContext> m_renderCtx{nullptr};

		SceneRendererSpecInfo m_specInfo{};

		RefPtr<render::Renderer2D> m_renderer2D{nullptr};

		#pragma region depth-pre
		gpu::PipelineHandle   m_depthPrePipeline{nullptr};
		gpu::RenderPassHandle m_depthPrePass{nullptr};

		gpu::RawImageHandle  m_depthPreAttachmentImage{nullptr};
		gpu::Texture2DHandle m_depthPreResolveAttachmentTexture{nullptr};

		gpu::RawImageHandle  m_geometryNormalsAttachmentImage{nullptr};
		gpu::Texture2DHandle m_geometryNormalsResolveAttachmentTexture{nullptr};

		gpu::RawImageHandle  m_geometryPositionsAttachmentImage{nullptr};
		gpu::Texture2DHandle m_geometryPositionsResolveAttachmentTexture{nullptr};
		#pragma endregion

		#pragma region ambient occlusion

		gpu::PipelineHandle   m_ssaoPipeline{nullptr};
		gpu::RenderPassHandle m_ssaoPass{nullptr};

		render::MaterialHandle m_aoFrameDataMaterial{nullptr};

		struct SSAOKernel
		{
			static constexpr uint32 c_SSAOSampleCount{64};
			tsm::float4             samples[c_SSAOSampleCount];

			SSAOKernel();
		};

		UniquePtr<SSAOKernel> m_ssaoKernel{nullptr};

		gpu::Texture2DHandle m_ssaoNoiseTexture{nullptr};
		gpu::Texture2DHandle m_ssaoOutputTexture{nullptr};

		gpu::ComputePipelineHandle m_aoBlurPipeline{nullptr};
		gpu::ComputePassHandle     m_aoBlurPass{nullptr};
		gpu::StorageImageHandle    m_aoBlurredOutputImage{nullptr};

		#pragma endregion

		#pragma region light culling
		gpu::ComputePipelineHandle m_lightCullingPipeline{nullptr};
		gpu::ComputePassHandle     m_lightCullingPass{nullptr};
		render::MaterialHandle     m_lightCullingMaterial{nullptr};

		gpu::StorageImageHandle m_computeImage{nullptr};

		#pragma endregion

		#pragma region skybox
		gpu::PipelineHandle    m_skyboxPipeline{nullptr};
		gpu::RenderPassHandle  m_skyboxPass{nullptr};
		render::MaterialHandle m_skyboxMaterial{nullptr};

		#pragma endregion

		#pragma region geometry
		gpu::PipelineHandle   m_geometryPipeline{nullptr};
		gpu::RenderPassHandle m_geometryPass{nullptr};
		#pragma endregion

		gpu::RawImageHandle  m_colourImage{nullptr};
		gpu::Texture2DHandle m_resolveColourTexture{nullptr};

		struct CameraUB
		{
			tsm::float4x4 view;
			tsm::float4x4 proj;
			tsm::float4x4 invProj;
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
			tsm::float4 cameraPos{0.0f, 0.0f, 0.0f, 1.0f};
		};

		gpu::UniformBufferPFFHandle m_sceneDataUBOs;
		gpu::UBOMappedDataPFF       m_mappedSceneDataUBOs;

		struct DrawCommand
		{
			render::MeshHandle mesh{nullptr};
			tsm::float4x4      transform{1.0f};
		};

		std::vector<DrawCommand> m_meshDrawCommands;
	};
}
