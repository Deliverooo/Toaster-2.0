#pragma once

#include <entt/entt.hpp>

#include "toaster/toast_lib/string.hpp"
#include "toaster/toast_render/renderer_2d.hpp"

namespace toaster
{
	class Entity;

	class Scene
	{
	public:
		Scene();
		~Scene();

		void onUpdate(float32 p_dt);
		void onRender(const RefPtr<Renderer2D> &p_renderer_2d, float32 p_dt);
		void setViewportSize(uint32 p_width, uint32 p_height);

		Entity createEntity(const U8String &p_name = u8"");
		void   destroyEntity(Entity p_entity);

		Entity getMainCameraEntity();

		entt::registry &      getRegistry();
		const entt::registry &getRegistry() const;

	private:
		template<typename Type>
		void onComponentAdded(Entity p_entity, Type &p_component);

		entt::registry m_registry;

		uint32 m_viewportWidth{0u};
		uint32 m_viewportHeight{0u};

		uint32 m_newEntityTagCount{0u};
		friend class Entity;
	};
}
