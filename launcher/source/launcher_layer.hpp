#pragma once

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
	class LauncherLayer final : public IAppLayer
	{
	public:
		LauncherLayer(Application *p_app);

		auto onInit() -> void override;
		auto onDestroy() -> void override;
		auto onUpdate(float32 p_dt) -> void override;
		auto onEvent(Event &p_event) -> void override;

	private:
		auto _onKeyPressEvent(KeyPressEvent &e) -> bool;

		uint32 m_viewportWidth{0u};
		uint32 m_viewportHeight{0u};

		RefPtr<gpu::VKPipeline>   m_fullscreenPipeline{nullptr};
		RefPtr<gpu::VKRenderPass> m_fullscreenRenderPass{nullptr};
		RefPtr<gpu::VKMaterial>   m_fullscreenMaterial{nullptr};

		UniquePtr<Renderer2D> m_renderer2D{nullptr};
	};
}
