#pragma once

#include <mesh.hpp>

#include "camera.hpp"
#include "layer.hpp"

#include "shader.hpp"

#include "mesh.hpp"

#include "vertex_array.hpp"

namespace toaster
{
	class ClientLayer : public IAppLayer
	{
	public:
		ClientLayer(Application *p_app_parent);
		~ClientLayer();

		void onInit() override;
		void onDestroy() override;
		void onUpdate(float32 p_dt) override;
		void onEvent(Event &p_event) override;

	private:
		RefPtr<gpu::Mesh> m_mesh;

		Camera m_camera;

		float32 m_lastX;
		float32 m_lastY;
		bool    m_firstMouse{true};
	};
}
