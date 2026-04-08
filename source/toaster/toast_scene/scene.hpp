#pragma once

#include <entt/entt.hpp>

#include "toast_lib/string.hpp"
#include "toast_render/renderer_2d.hpp"

namespace toaster
{
	class Entity;

	class Scene
	{
	public:
		Scene(gpu::VKGPUContext *p_ctx, const String &p_name = "");
		~Scene();

		void onUpdate(float32 p_dt);

		void onRender(vk::raii::CommandBuffer &p_cmd, uint32 p_frame_index, float32 p_dt, const RefPtr<Renderer2D> &p_renderer_2d);
		void onRender(vk::raii::CommandBuffer &p_cmd, uint32 p_frame_index, float32 p_dt, const RefPtr<Renderer2D> &p_renderer_2d, const glm::mat4 &p_view,
					  const glm::mat4 &        p_projection);

		void setViewportSize(uint32 p_width, uint32 p_height);

		Entity createEntity(const String &p_name = "");
		void   destroyEntity(Entity p_entity);

		Entity getMainCameraEntity();

		entt::registry &                    getRegistry();
		[[nodiscard]] const entt::registry &getRegistry() const;

		void                 setName(const String &p_name);
		[[nodiscard]] String getName() const;

	private:
		template<typename Type>
		void onComponentAdded(Entity p_entity, Type &p_component);

		gpu::VKGPUContext *m_ctx{nullptr};

		entt::registry m_registry;

		String m_name;

		uint32 m_viewportWidth{0u};
		uint32 m_viewportHeight{0u};

		uint32 m_newEntityTagCount{0u};
		friend class Entity;
		friend class SceneSerializer;
		friend class SceneRenderer;
	};
}
