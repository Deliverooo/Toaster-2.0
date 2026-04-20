#pragma once

#include "editor_camera.hpp"
#include "toaster/toast_kernel/layer.hpp"
#include "toaster/toast_render/renderer_2d.hpp"

#include "toaster/toast_lib/events/key_event.hpp"
#include "toaster/toast_lib/events/mouse_event.hpp"
#include "toaster/toast_lib/events/window_event.hpp"

#include "toast_gpu/vk/vk_gpu_context.hpp"
#include "toast_gpu/vk/vk_vertex_buffer.hpp"
#include "toast_gpu/vk/vk_index_buffer.hpp"
#include "toast_gpu/vk/vk_material.hpp"
#include "toast_gpu/vk/vk_shader.hpp"
#include "toast_gpu/vk/vk_mesh.hpp"
#include "toast_gpu/vk/vk_pipeline.hpp"
#include "toast_gpu/vk/vk_render_pass.hpp"
#include "toast_gpu/vk/vk_texture.hpp"
#include "toast_gpu/vk/vk_uniform_buffer.hpp"
#include "toast_gpu/vk/vk_render_attachment.hpp"
#include "toast_scene/scene_renderer.hpp"


namespace toaster
{
	class ClientLayer final : public IAppLayer
	{
	public:
		ClientLayer(Application *p_app);

		auto onInit() -> void override;
		auto onDestroy() -> void override;
		auto onUpdate(float32 p_dt) -> void override;
		auto onEvent(Event &p_event) -> void override;

		auto onUIRender() -> void override;

	private:
		auto _onKeyPressEvent(KeyPressEvent &e) -> bool;
		auto _onWindowResizeEvent(WindowResizeEvent &e) -> bool;

		float32 m_time{0.0f};

		uint32 m_viewportWidth{0u};
		uint32 m_viewportHeight{0u};

		RefPtr<Renderer2D> m_renderer2D{nullptr};
	};
}
