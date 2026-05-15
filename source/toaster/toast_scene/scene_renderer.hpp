#pragma once

#include "scene.hpp"
#include "toast_gpu/vk/vk_compute_pass.hpp"
#include "toast_gpu/vk/vk_compute_pipeline.hpp"
#include "toast_gpu/vk/vk_render_pass.hpp"
#include "toast_render/mesh.hpp"

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

		auto begin(uint32 p_frame_index, const glm::mat4 &p_view_matrix, const glm::mat4 &p_projection_matrix) -> void;
		auto end(gpu::VKCommandBuffer &p_cmd, uint32 p_frame_index) -> void;
		auto renderMesh(const render::MeshHandle &p_mesh, const glm::mat4 &p_transform) -> void;

		auto getSpecInfo() const -> const SceneRendererSpecInfo &;
		auto getOutputColourTexture() const -> const gpu::Texture2DHandle &;
		auto getOutputDepthTexture() const -> const gpu::Texture2DHandle &;

		auto getOutputComputeImage() const -> const RefPtr<gpu::VKImage2D> &;

		auto getGeometryPositionsTexture() const -> const gpu::Texture2DHandle &;
		auto getGeometryNormalsTexture() const -> const gpu::Texture2DHandle &;

		auto getRenderer2D() -> RefPtr<render::Renderer2D>;

		auto onResize(uint32 p_width, uint32 p_height) -> void;
		auto setEnvironmentBackground(const gpu::Texture3DHandle &p_texture) -> void;

	private:
		auto _renderDepthPrePass(gpu::VKCommandBuffer &p_cmd, uint32 p_frame_index) -> void;
		auto _renderLightCullingPass(gpu::VKCommandBuffer &p_cmd, uint32 p_frame_index) -> void;
		auto _renderSkyboxPass(gpu::VKCommandBuffer &p_cmd, uint32 p_frame_index) -> void;
		auto _renderGeometryPass(gpu::VKCommandBuffer &p_cmd, uint32 p_frame_index) -> void;

		NonOwningPtr<render::RenderContext> m_renderCtx{nullptr};

		SceneRendererSpecInfo m_specInfo{};

		RefPtr<render::Renderer2D> m_renderer2D{nullptr};

		#pragma region depth-pre
		gpu::PipelineHandle   m_depthPrePipeline{nullptr};
		gpu::RenderPassHandle m_depthPrePass{nullptr};

		gpu::Texture2DHandle m_depthPreAttachmentTexture{nullptr};
		#pragma endregion

		#pragma region light culling
		gpu::ComputePipelineHandle m_lightCullingPipeline{nullptr};
		gpu::ComputePassHandle     m_lightCullingPass{nullptr};
		render::MaterialHandle     m_lightCullingMaterial{nullptr};

		gpu::Image2DHandle m_computeImage{nullptr};

		#pragma endregion

		#pragma region skybox
		gpu::PipelineHandle    m_skyboxPipeline{nullptr};
		gpu::RenderPassHandle  m_skyboxPass{nullptr};
		render::MaterialHandle m_skyboxMaterial{nullptr};
		gpu::Texture3DHandle   m_skyboxMap{nullptr};
		bool                   m_reloadSkybox{false};
		#pragma endregion

		#pragma region geometry
		gpu::PipelineHandle   m_geometryPipeline{nullptr};
		gpu::RenderPassHandle m_geometryPass{nullptr};

		gpu::Texture2DHandle m_geometryPositionsAttachmentTexture{nullptr};
		gpu::Texture2DHandle m_geometryNormalsAttachmentTexture{nullptr};
		#pragma endregion

		gpu::Texture2DHandle m_outputColourTexture{nullptr};

		struct CameraUB
		{
			glm::mat4 view;
			glm::mat4 proj;
		};

		gpu::UniformBufferPFFHandle m_cameraUBOs;
		std::vector<void *>         m_mappedCameraUBOs;

		struct DirectionalLightUB
		{
			static constexpr uint32 c_maxDirectionalLights{4u};

			uint32           count{0u};
			glm::vec3        _padding{0.0f};
			DirectionalLight directionalLights[c_maxDirectionalLights]{};
		};

		gpu::UniformBufferPFFHandle m_directionalLightUBOs;
		std::vector<void *>         m_mappedDirectionalLightUBOs;

		struct PointLightUB
		{
			static constexpr uint32 c_maxPointLights{128u};

			uint32     count{0u};
			glm::vec3  _padding{0.0f};
			PointLight pointLights[c_maxPointLights]{};
		};

		gpu::UniformBufferPFFHandle m_pointLightUBOs;
		std::vector<void *>         m_mappedPointLightUBOs;

		struct SceneDataUB
		{
			glm::vec4 cameraPos{0.0f, 0.0f, 0.0f, 1.0f};
		};

		gpu::UniformBufferPFFHandle m_sceneDataUBOs;
		std::vector<void *>         m_mappedSceneDataUBOs;

		struct DrawCommand
		{
			render::MeshHandle mesh{nullptr};
			glm::mat4          transform{1.0f};
		};

		std::vector<DrawCommand> m_meshDrawCommands;
	};
}
