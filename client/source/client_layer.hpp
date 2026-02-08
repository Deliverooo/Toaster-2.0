#pragma once

#include "layer.hpp"
#include "mesh.hpp"
#include "ortho_camera.hpp"

#include "shader.hpp"
#include "texture.hpp"

#include "vertex_array.hpp"

namespace toaster
{
	class ClientLayer : public IAppLayer
	{
	public:
		ClientLayer(Application *p_app);
		~ClientLayer() override;

		void onInit() override;
		void onDestroy() override;
		void onUpdate(float32 p_dt) override;
		void onEvent(Event &p_event) override;

	private:
		RefPtr<gpu::Texture2D> m_texture;

		OrthoCamera m_camera;

		float32 m_time{0.0f};
	};
}
