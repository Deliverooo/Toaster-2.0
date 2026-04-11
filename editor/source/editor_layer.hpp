#pragma once

#include "toast_kernel/application.hpp"

#include "toast_gpu/vk/vk_image.hpp"
#include "toast_gpu/vk/vk_mesh.hpp"
#include "toast_gpu/vk/vk_render_pass.hpp"
#include "toast_gpu/vk/vk_texture.hpp"

namespace toaster
{
	class EditorLayer final : public IAppLayer
	{
	public:
		EditorLayer(Application *p_app);

		void onInit() override;
		void onDestroy() override;
		void onUpdate(float32 p_dt) override;
		void onEvent(Event &p_event) override;

		void onUIRender() override;

	private:
		uint32 m_viewportWidth{0u};
		uint32 m_viewportHeight{0u};

		RefPtr<gpu::VKPipeline>   m_fullscreenPipeline{nullptr};
		RefPtr<gpu::VKRenderPass> m_fullscreenPass{nullptr};
		RefPtr<gpu::VKMaterial>   m_fullscreenMaterial{nullptr};

		RefPtr<gpu::VKTexture2D> m_texture{nullptr};

		float32 m_time{0.0f};

		struct FrameDataUB
		{
			glm::vec2 res{1.0f};
			float32   time{0.0f};
		};

		RefPtr<gpu::VKUniformBufferPFF> m_frameDataUBOs{nullptr};
	};
}
