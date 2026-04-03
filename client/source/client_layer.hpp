#pragma once

#include "toaster/toast_kernel/layer.hpp"
#include "toaster/toast_render/renderer_2d.hpp"

#include "toaster/toast_gpu/shader.hpp"
#include "toaster/toast_gpu/texture.hpp"
#include "toaster/toast_kernel/ortho_camera_controller.hpp"

#include "toaster/toast_lib/events/key_event.hpp"
#include "toaster/toast_lib/events/mouse_event.hpp"
#include "toaster/toast_lib/events/window_event.hpp"

#include "toast_gpu/vk/vk_gpu_context.hpp"
#include "toast_gpu/vk/vk_pipeline.hpp"

namespace toaster
{
	class ClientLayer final : public IAppLayer
	{
	public:
		ClientLayer(Application *p_app);

		void onInit() override;
		void onDestroy() override;
		void onUpdate(float32 p_dt) override;
		void onEvent(Event &p_event) override;

	private:
		bool onKeyPressEvent(KeyPressEvent &e);

		void _recordCommandBuffer(uint32 p_image_index);

		RefPtr<gpu::VKPipeline> m_pipeline{nullptr};

		struct Vertex
		{
			glm::vec3 position;
			glm::vec3 colour;
		};

		const std::vector<Vertex> m_vertices = {
			{{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 1.0f}}
		};

		vk::raii::Buffer       m_vertexBuffer{nullptr};
		vk::raii::DeviceMemory m_vertexBufferMemory{nullptr};
	};
}
