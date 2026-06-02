#pragma once

#include "fp_camera.hpp"
#include "toast_kernel/layer.hpp"

#include "toast_lib/events/key_event.hpp"

#include "toast_scene/scene_renderer.hpp"
#include "toast_script/script_engine.hpp"

namespace toaster
{
	class RuntimeLayer final : public IAppLayer
	{
	public:
		auto onInit() -> void override;
		auto onUpdate(float32 p_dt) -> void override;
		auto onResize(tsm::uint2 p_size) -> void override;;
		auto onEvent(Event &p_event) -> void override;

	private:
		auto _onKeyPressEvent(KeyPressEvent &e) -> bool;

		tsm::uint2 m_viewportSize{0u};

		UniquePtr<script::ScriptEngine> m_scriptEngine{nullptr};

		UniquePtr<Scene>         m_scene{nullptr};
		UniquePtr<SceneRenderer> m_sceneRenderer{nullptr};

		gpu::PipelineHandle   m_fullscreenPipeline{nullptr};
		render::RenderPassHandle m_fullscreenRenderPass{nullptr};

		FPCamera m_camera;
	};
}
