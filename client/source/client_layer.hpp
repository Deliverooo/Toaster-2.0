#pragma once

#include "layer.hpp"

#include "shader.hpp"

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
		RefPtr<gpu::Shader> m_shader;

		RefPtr<gpu::VertexArray> m_vao;
	};
}
