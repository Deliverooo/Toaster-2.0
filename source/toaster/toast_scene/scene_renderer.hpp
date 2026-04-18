#pragma once

#include "scene.hpp"
#include "toast_gpu/vk/vk_mesh.hpp"

namespace toaster
{
	struct SceneRendererSpecInfo
	{
		RefPtr<Scene> scene{nullptr};
		uint32        viewportWidth{0u};
		uint32        viewportHeight{0u};
	};

	class SceneRenderer
	{
	public:
		SceneRenderer(gpu::VKGPUContext *p_ctx, const SceneRendererSpecInfo &p_spec_info);
		~SceneRenderer();

		auto begin(const vk::raii::CommandBuffer &p_cmd, uint32 p_frame_index, const glm::mat4 &p_view_matrix, const glm::mat4 &p_projection_matrix) -> void;
		auto end(const vk::raii::CommandBuffer &p_cmd, uint32 p_frame_index) -> void;
		auto renderMesh(RefPtr<gpu::VKMesh> p_mesh, const glm::mat4 &p_transform) -> void;

		auto getSpecInfo() const -> const SceneRendererSpecInfo &;
		auto getOutputColourTexture() const -> const RefPtr<gpu::VKTexture2D> &;
		auto getOutputDepthImage() const -> const RefPtr<gpu::VKImage2D> &;

		auto getRenderer2D() -> RefPtr<Renderer2D>;

		auto onResize(uint32 p_width, uint32 p_height) -> void;
		auto setEnvironmentBackground(RefPtr<gpu::VKTexture2D> p_texture) -> void;

	private:
		auto _renderSkyboxPass(const vk::raii::CommandBuffer &p_cmd, uint32 p_frame_index) -> void;
		auto _renderGeometryPass(const vk::raii::CommandBuffer &p_cmd, uint32 p_frame_index) -> void;

		gpu::VKGPUContext *m_ctx{nullptr};

		SceneRendererSpecInfo m_specInfo{};

		RefPtr<Renderer2D> m_renderer2D{nullptr};

		RefPtr<gpu::VKPipeline>   m_skyboxPipeline{nullptr};
		RefPtr<gpu::VKRenderPass> m_skyboxPass{nullptr};
		RefPtr<gpu::VKMaterial>   m_skyboxMaterial{nullptr};
		RefPtr<gpu::VKTexture2D>  m_skyboxTexture{nullptr};

		RefPtr<gpu::VKPipeline>   m_geometryPipeline{nullptr};
		RefPtr<gpu::VKRenderPass> m_geometryPass{nullptr};

		RefPtr<gpu::VKImage2D>   m_MSAAGeometryPositionsAttachmentImage{nullptr};
		RefPtr<gpu::VKTexture2D> m_resolveGeometryPositionsAttachmentTexture{nullptr};

		RefPtr<gpu::VKImage2D>   m_MSAAGeometryNormalsAttachmentImage{nullptr};
		RefPtr<gpu::VKTexture2D> m_resolveGeometryNormalsAttachmentTexture{nullptr};

		RefPtr<gpu::VKImage2D>   m_MSAAColourAttachmentImage{nullptr};
		RefPtr<gpu::VKTexture2D> m_resolveOutputColourTexture{nullptr};

		RefPtr<gpu::VKImage2D> m_MSAADepthAttachmentImage{nullptr};
		RefPtr<gpu::VKImage2D> m_resolveOutputDepthImage{nullptr};

		struct CameraUB
		{
			glm::mat4 view;
			glm::mat4 proj;
		};

		RefPtr<gpu::VKUniformBufferPFF> m_cameraUBOs;
		std::vector<void *>             m_mappedCameraUBOs;

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
			glm::vec3 cameraPos{0.0f};
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
