#pragma once

#include "layer.hpp"
#include "ref_ptr.hpp"
#include "gpu/material.hpp"
#include "gpu/render_pass.hpp"
#include "gpu/texture.hpp"
#include "nvrhi/nvrhi.h"
#include "renderer/renderer_2d.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace tst
{
	class SandboxLayer : public ILayer
	{
	public:
		SandboxLayer();
		~SandboxLayer() override;

		void onInit() override;

		void onDestroy() override;

		void onUpdate(float32 dt) override;

		void onEvent(Event &event) override;

		void onGUIRender() override;

	private:
		RefPtr<Renderer2D> m_renderer2d;

		RefPtr<Texture2D> m_texture;
		RefPtr<Framebuffer> m_framebuffer;
	};
}
