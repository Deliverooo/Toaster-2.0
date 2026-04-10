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

		void begin(const vk::raii::CommandBuffer &p_cmd, uint32 p_frame_index, const glm::mat4 &p_view_matrix, const glm::mat4 &p_projection_matrix);
		void end(const vk::raii::CommandBuffer &p_cmd, uint32 p_frame_index);

		void renderMesh(const RefPtr<gpu::VKMesh> &p_mesh, const glm::mat4 &p_transform);

		void onResize(uint32 p_width, uint32 p_height);

		const SceneRendererSpecInfo &getSpecInfo() const;

		const RefPtr<gpu::VKTexture2D> &getOutputColourTexture() const;
		const RefPtr<gpu::VKImage2D> &  getOutputDepthImage() const;

	private:
		void _renderGeometryPass(const vk::raii::CommandBuffer &p_cmd, uint32 p_frame_index);

		gpu::VKGPUContext *m_ctx{nullptr};

		SceneRendererSpecInfo m_specInfo{};
		RefPtr<gpu::VKPipeline>   m_geometryPipeline{nullptr};
		RefPtr<gpu::VKRenderPass> m_geometryPass{nullptr};

		RefPtr<gpu::VKImage2D> m_MSAAColourAttachmentImage{nullptr};
		RefPtr<gpu::VKImage2D> m_MSAADepthAttachmentImage{nullptr};

		RefPtr<gpu::VKTexture2D> m_resolveOutputColourTexture{nullptr};
		RefPtr<gpu::VKImage2D>   m_resolveOutputDepthImage{nullptr};

		struct CameraUB
		{
			glm::mat4 view;
			glm::mat4 proj;
		};

		RefPtr<gpu::VKUniformBufferPFF> m_cameraUBOs;
		std::vector<void *>             m_mappedCameraUBOs;

		struct DrawCommand
		{
			RefPtr<gpu::VKMesh> mesh{nullptr};
			glm::mat4           transform{1.0f};
		};

		std::vector<DrawCommand> m_meshDrawCommands;
	};
}
