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
	class ClientLayer : public IAppLayer
	{
	public:
		ClientLayer(Application *p_app);

		void onInit() override;
		void onDestroy() override;
		void onUpdate(float32 p_dt) override;
		void onEvent(Event &p_event) override;

	private:
		bool onKeyPressEvent(KeyPressEvent &e);
		bool onMouseMoveEvent(MouseMoveEvent &e);
		bool onWindowResizeEvent(WindowResizeEvent &e);

		RefPtr<Renderer2D> m_renderer2d;

		RefPtr<gpu::ITexture2D> m_texture;
		RefPtr<gpu::ITexture2D> m_texture2;

		OrthoCameraController m_cameraController;
		float32 m_time{0.0f};
	};
}
