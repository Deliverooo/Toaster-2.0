#pragma once

#include <entt/entt.hpp>
#include "components.hpp"

#include "toaster/toast_render/renderer_2d.hpp"

namespace toaster
{
	class Entity;

	class Scene
	{
	public:
		Scene();
		~Scene();

		void onUpdate(const RefPtr<Renderer2D> &p_renderer_2d, float32 p_dt);

		[[nodiscard]] Entity createEntity(const std::wstring& p_name = L"");

	private:
		entt::registry m_registry;
		friend class Entity;
	};
}
