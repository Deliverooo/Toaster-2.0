#pragma once

#include "fp_camera.hpp"
#include "toaster/toast_kernel/layer.hpp"
#include "toaster/toast_render/renderer_2d.hpp"

#include "toaster/toast_lib/events/key_event.hpp"
#include "toaster/toast_lib/events/mouse_event.hpp"
#include "toaster/toast_lib/events/window_event.hpp"

#include "toast_gpu/vk/vk_vertex_buffer.hpp"
#include "toast_scene/scene_renderer.hpp"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

#include "toast_render/shader_library.hpp"
#include "toast_scripting/script_common.hpp"
#include "toast_scripting/script_engine.hpp"
#include "toast_scripting/script_object.hpp"

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

		auto onUIRender() -> void override;

	private:
		auto _onKeyPressEvent(KeyPressEvent &e) -> bool;
		auto _onWindowResizeEvent(WindowResizeEvent &e) -> bool;

		float32 m_time{0.0f};

		uint32 m_viewportWidth{0u};
		uint32 m_viewportHeight{0u};

		UniquePtr<script::ScriptEngine> m_scriptEngine{nullptr};

		RefPtr<Scene>         m_scene{nullptr};
		RefPtr<SceneRenderer> m_sceneRenderer{nullptr};

		ShaderLibrary m_shaderLibrary;

		RefPtr<gpu::VKPipeline>   m_fullscreenPipeline{nullptr};
		RefPtr<gpu::VKRenderPass> m_fullscreenRenderPass{nullptr};

		FPCamera m_camera;
	};
}
