#pragma once

#include "toast_kernel/layer.hpp"

#include "toast_lib/events/key_event.hpp"

#include "toast_scene/scene_renderer.hpp"
#include "toast_script/script_engine.hpp"

namespace toaster
{
	class RuntimeLayer final : public IAppLayer
	{
	public:
		RuntimeLayer(Application *p_app);

		auto onInit() -> void override;
		auto onDestroy() -> void override;
		auto onUpdate(float32 p_dt) -> void override;
		auto onEvent(Event &p_event) -> void override;

	private:
		auto _onKeyPressEvent(KeyPressEvent &e) -> bool;

		uint32 m_viewportWidth{0u};
		uint32 m_viewportHeight{0u};

		UniquePtr<script::ScriptEngine> m_scriptEngine{nullptr};

		RefPtr<Scene>         m_scene{nullptr};
		RefPtr<SceneRenderer> m_sceneRenderer{nullptr};

		gpu::PipelineHandle   m_fullscreenPipeline{nullptr};
		gpu::RenderPassHandle m_fullscreenRenderPass{nullptr};
	};
}
