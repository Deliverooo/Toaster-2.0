#pragma once

#include "toaster/toast_kernel/layer.hpp"
#include "toaster/toast_render/renderer_2d.hpp"

#include "toaster/toast_gpu/shader.hpp"
#include "toaster/toast_gpu/texture.hpp"
#include "toaster/toast_kernel/ortho_camera_controller.hpp"

#include "toaster/toast_lib/events/key_event.hpp"
#include "toaster/toast_lib/events/mouse_event.hpp"
#include "toaster/toast_lib/events/window_event.hpp"
#include "toast_render/material.hpp"

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

		RefPtr<gpu::IShader> m_shader;
		RefPtr<Material>     m_material;

		float32 m_time{0.0f};
	};
}
