#pragma once

#include "scene.hpp"
#include "toast_gpu/vk/vk_compute_pass.hpp"
#include "toast_gpu/vk/vk_compute_pipeline.hpp"
#include "toast_gpu/vk/vk_mesh.hpp"

namespace toaster
{
	struct TST_API SceneRendererSpecInfo
	{
		io::filesystem::Path resourceDirectory{};
		NonOwningPtr<Scene>  scene{nullptr};
		uint32               viewportWidth{0u};
		uint32               viewportHeight{0u};
		int32                viewportOffsetX{0u};
		int32                viewportOffsetY{0u};
	};

	class TST_API SceneRenderer
	{
	public:
		SceneRenderer(render::RenderContext *p_render_ctx, const SceneRendererSpecInfo &p_spec_info);
		~SceneRenderer();

		auto begin( uint32 p_frame_index, const glm::mat4 &p_view_matrix, const glm::mat4 &p_projection_matrix) -> void;
		auto end( gpu::VKCommandBuffer &p_cmd, uint32 p_frame_index) -> void;
		auto renderMesh(RefPtr<gpu::VKMesh> p_mesh, const glm::mat4 &p_transform) -> void;

		auto getSpecInfo() const -> const SceneRendererSpecInfo &;
		auto getOutputColourTexture() const -> const RefPtr<gpu::VKTexture2D> &;
		auto getOutputDepthTexture() const -> const RefPtr<gpu::VKTexture2D> &;

		auto getOutputComputeImage() const -> const RefPtr<gpu::VKImage2D> &;

		auto getGeometryPositionsTexture() const -> const RefPtr<gpu::VKTexture2D> &;
		auto getGeometryNormalsTexture() const -> const RefPtr<gpu::VKTexture2D> &;

		auto getRenderer2D() -> RefPtr<render::Renderer2D>;

		auto onResize(uint32 p_width, uint32 p_height) -> void;
		auto setEnvironmentBackground(const RefPtr<gpu::VKTexture3D> &p_texture) -> void;

	private:
		auto _renderDepthPrePass( gpu::VKCommandBuffer &p_cmd, uint32 p_frame_index) -> void;
		auto _renderLightCullingPass( gpu::VKCommandBuffer &p_cmd, uint32 p_frame_index) -> void;
		auto _renderSkyboxPass( gpu::VKCommandBuffer &p_cmd, uint32 p_frame_index) -> void;
		auto _renderGeometryPass( gpu::VKCommandBuffer &p_cmd, uint32 p_frame_index) -> void;

		NonOwningPtr<render::RenderContext> m_renderCtx{nullptr};

		SceneRendererSpecInfo m_specInfo{};

		RefPtr<render::Renderer2D> m_renderer2D{nullptr};

		#pragma region depth-pre
		RefPtr<gpu::VKPipeline>   m_depthPrePipeline{nullptr};
		RefPtr<gpu::VKRenderPass> m_depthPrePass{nullptr};

		RefPtr<gpu::VKTexture2D> m_depthPreAttachmentTexture{nullptr};
		#pragma endregion

		#pragma region light culling
		// RefPtr<gpu::VKShader>          m_lightCullingShader{nullptr};
		RefPtr<gpu::VKComputePipeline> m_lightCullingPipeline{nullptr};
		RefPtr<gpu::VKComputePass>     m_lightCullingPass{nullptr};
		RefPtr<gpu::VKMaterial>        m_lightCullingMaterial{nullptr};

		RefPtr<gpu::VKImage2D> m_computeImage{nullptr};

		#pragma endregion

		#pragma region skybox
		RefPtr<gpu::VKPipeline>   m_skyboxPipeline{nullptr};
		RefPtr<gpu::VKRenderPass> m_skyboxPass{nullptr};
		RefPtr<gpu::VKMaterial>   m_skyboxMaterial{nullptr};
		RefPtr<gpu::VKTexture3D>  m_skyboxMap{nullptr};
		bool                      m_reloadSkybox{false};
		#pragma endregion

		#pragma region geometry
		RefPtr<gpu::VKPipeline>   m_geometryPipeline{nullptr};
		RefPtr<gpu::VKRenderPass> m_geometryPass{nullptr};

		RefPtr<gpu::VKTexture2D> m_geometryPositionsAttachmentTexture{nullptr};
		RefPtr<gpu::VKTexture2D> m_geometryNormalsAttachmentTexture{nullptr};
		#pragma endregion

		RefPtr<gpu::VKTexture2D> m_outputColourTexture{nullptr};

		struct CameraUB
		{
			glm::mat4 view;
			glm::mat4 proj;
		};

		RefPtr<gpu::VKUniformBufferPFF> m_cameraUBOs;
		std::vector<void *>             m_mappedCameraUBOs;

		struct DirectionalLightUB
		{
			static constexpr uint32 c_maxDirectionalLights{4u};

			uint32           count{0u};
			glm::vec3        _padding{0.0f};
			DirectionalLight directionalLights[c_maxDirectionalLights]{};
		};

		RefPtr<gpu::VKUniformBufferPFF> m_directionalLightUBOs;
		std::vector<void *>             m_mappedDirectionalLightUBOs;

		struct PointLightUB
		{
			static constexpr uint32 c_maxPointLights{128u};

			uint32     count{0u};
			glm::vec3  _padding{0.0f};
			PointLight pointLights[c_maxPointLights]{};
		};

		RefPtr<gpu::VKUniformBufferPFF> m_pointLightUBOs;
		std::vector<void *>             m_mappedPointLightUBOs;

		struct SceneDataUB
		{
			glm::vec4 cameraPos{0.0f, 0.0f, 0.0f, 1.0f};
		};

		RefPtr<gpu::VKUniformBufferPFF> m_sceneDataUBOs;
		std::vector<void *>             m_mappedSceneDataUBOs;

		struct DrawCommand
		{
			RefPtr<gpu::VKMesh> mesh{nullptr};
			glm::mat4           transform{1.0f};
		};

		std::vector<DrawCommand> m_meshDrawCommands;
	};
}
