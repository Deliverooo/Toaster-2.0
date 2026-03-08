#pragma once

#include "toaster/toast_kernel/layer.hpp"
#include "toaster/toast_render/renderer_2d.hpp"

#include "toaster/toast_gpu/shader.hpp"
#include "toaster/toast_gpu/texture.hpp"
#include "toaster/toast_kernel/ortho_camera_controller.hpp"

#include "toaster/toast_lib/events/key_event.hpp"
#include "toaster/toast_lib/events/mouse_event.hpp"
#include "toaster/toast_lib/events/window_event.hpp"

namespace toaster
{
	class EditorLayer : public IAppLayer
	{
	public:
		EditorLayer(Application *p_app);

		void onInit() override;
		void onDestroy() override;
		void onUpdate(float32 p_dt) override;
		void onEvent(Event &p_event) override;

		void onUIRender() override;

	private:
		bool onKeyPressEvent(KeyPressEvent &p_event);
		bool onMouseMoveEvent(MouseMoveEvent &p_event);
		bool onWindowResizeEvent(WindowResizeEvent &p_event);

		RefPtr<Renderer2D> m_renderer2d;

		RefPtr<gpu::Texture2D> m_texture;
		RefPtr<gpu::Texture2D> m_texture2;

		OrthoCameraController m_cameraController;
		float32 m_time{0.0f};
	};
}
