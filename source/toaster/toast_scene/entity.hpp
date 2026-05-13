#pragma once


#include <entt/entt.hpp>
#include "scene.hpp"

namespace toaster
{
	class TST_API Entity
	{
	public:
		Entity() = default;

		Entity(entt::entity p_handle, Scene *p_scene) : m_handle(p_handle), m_scene(p_scene)
		{
		}

		~Entity() = default;

		template<typename Type, typename... TArgs>
		auto addComponent(TArgs &&... p_args) -> Type &
		{
			TST_PERMA_ASSERT_MSG(!hasComponent<Type>(), "Entity already has component!");
			Type &comp = m_scene->m_registry.emplace<Type>(m_handle, std::forward<TArgs>(p_args)...);
			m_scene->onComponentAdded<Type>(*this, comp);
			return comp;
		}

		template<typename Type>
		auto getComponent() -> Type &
		{
			TST_PERMA_ASSERT_MSG(hasComponent<Type>(), "Entity doesn't have component!");
			return m_scene->m_registry.get<Type>(m_handle);
		}

		template<typename Type>
		auto getComponent() const -> const Type &
		{
			TST_PERMA_ASSERT_MSG(hasComponent<Type>(), "Entity doesn't have component!");
			return m_scene->m_registry.get<Type>(m_handle);
		}

		// returns nullptr if entity does not have the requested component type
		template<typename Type>
		auto tryGetComponent() -> Type *
		{
			return m_scene->m_registry.try_get<Type>(m_handle);
		}

		// returns nullptr if entity does not have the requested component type
		template<typename Type>
		auto tryGetComponent() const -> const Type *
		{
			return m_scene->m_registry.try_get<Type>(m_handle);
		}

		template<typename... Type>
		auto hasComponent() -> bool
		{
			return m_scene->m_registry.all_of<Type...>(m_handle);
		}

		template<typename... Type>
		[[nodiscard]] auto hasComponent() const -> bool
		{
			return m_scene->m_registry.all_of<Type...>(m_handle);
		}

		template<typename... Type>
		auto hasAny() -> bool
		{
			return m_scene->m_registry.any_of<Type...>(m_handle);
		}

		template<typename... Type>
		[[nodiscard]] auto hasAny() const -> bool
		{
			return m_scene->m_registry.any_of<Type...>(m_handle);
		}

		template<typename Type>
		auto removeComponent() -> void
		{
			TST_PERMA_ASSERT_MSG(hasComponent<Type>(), "Entity doesn't have component!");
			m_scene->m_registry.remove<Type>(m_handle);
		}

		template<typename Type>
		auto removeComponentIfExists() -> void
		{
			if (hasComponent<Type>())
				m_scene->m_registry.remove<Type>(m_handle);
		}

		auto isValid() const -> bool { return (m_handle != entt::null) && m_scene && m_scene->m_registry.valid(m_handle); }

		operator bool() const { return isValid(); }
		operator uint32() const { return static_cast<uint32>(m_handle); }
		operator entt::entity() const { return m_handle; }

		auto operator==(Entity p_entity) const -> bool { return m_handle == p_entity.m_handle; }

	private:
		entt::entity m_handle{entt::null};
		Scene *      m_scene{nullptr};
	};
}
