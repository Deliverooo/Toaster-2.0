#pragma once

#include "scene.hpp"
#include "toast_render/mesh.hpp"

namespace toaster
{
	struct TST_SCENE_API SceneRendererSpecInfo
	{
		io::filesystem::Path resourceDirectory{};
		NonOwningPtr<Scene>  scene{nullptr};
		uint32               viewportWidth{0u};
		uint32               viewportHeight{0u};
		int32                viewportOffsetX{0u};
		int32                viewportOffsetY{0u};
	};

	class TST_SCENE_API SceneRenderer
	{
	public:
		SceneRenderer(render::RenderContext *p_render_ctx, const SceneRendererSpecInfo &p_spec_info);
		~SceneRenderer();

		auto begin(const glm::mat4 &p_view_matrix, const glm::mat4 &p_projection_matrix) -> void;
		auto end(gpu::VKCommandBuffer *p_cmd) -> void;
		auto renderMesh(const render::MeshHandle &p_mesh, const glm::mat4 &p_transform) -> void;

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

		auto onResize(uint32 p_width, uint32 p_height) -> void;

		auto reloadEnvironmentMaps(const gpu::Texture3DHandle &p_skybox, const gpu::Texture3DHandle &p_diffuse_irradiance) -> void;

	private:
		auto _renderDepthPrePass(gpu::VKCommandBuffer *p_cmd) -> void;
		auto _renderAOPass(gpu::VKCommandBuffer *p_cmd) -> void;
		auto _renderLightCullingPass(gpu::VKCommandBuffer *p_cmd) -> void;
		auto _renderSkyboxPass(gpu::VKCommandBuffer *p_cmd) -> void;
		auto _renderGeometryPass(gpu::VKCommandBuffer *p_cmd) -> void;

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
			glm::vec4               samples[c_SSAOSampleCount];

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
			glm::mat4 view;
			glm::mat4 proj;
			glm::mat4 invProj;
		};

		gpu::UniformBufferPFFHandle m_cameraUBOs;
		gpu::UBOMappedDataPFF       m_mappedCameraUBOs;

		struct DirectionalLightUB
		{
			static constexpr uint32 c_maxDirectionalLights{4u};

			uint32           count{0u};
			glm::vec3        _padding{0.0f};
			DirectionalLight directionalLights[c_maxDirectionalLights]{};
		};

		gpu::UniformBufferPFFHandle m_directionalLightUBOs;
		gpu::UBOMappedDataPFF       m_mappedDirectionalLightUBOs;

		struct PointLightUB
		{
			static constexpr uint32 c_maxPointLights{128u};

			uint32     count{0u};
			glm::vec3  _padding{0.0f};
			PointLight pointLights[c_maxPointLights]{};
		};

		gpu::UniformBufferPFFHandle m_pointLightUBOs;
		gpu::UBOMappedDataPFF       m_mappedPointLightUBOs;

		struct SceneDataUB
		{
			glm::vec4 cameraPos{0.0f, 0.0f, 0.0f, 1.0f};
		};

		gpu::UniformBufferPFFHandle m_sceneDataUBOs;
		gpu::UBOMappedDataPFF       m_mappedSceneDataUBOs;

		struct DrawCommand
		{
			render::MeshHandle mesh{nullptr};
			glm::mat4          transform{1.0f};
		};

		std::vector<DrawCommand> m_meshDrawCommands;
	};
}
