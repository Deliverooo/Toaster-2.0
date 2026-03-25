#pragma once

#include <entt/entt.hpp>
#include "scene.hpp"

namespace toaster
{
	class Entity
	{
	public:
		Entity() = default;

		Entity(entt::entity handle, Scene *scene) : m_handle(handle), m_scene(scene)
		{
		}

		~Entity() = default;

		template<typename Type, typename... TArgs>
		Type &addComponent(TArgs &&... args)
		{
			TST_ASSERT_MSG(!hasComponent<Type>(), "Entity already has component!");
			Type &comp = m_scene->m_registry.emplace<Type>(m_handle, std::forward<TArgs>(args)...);
			m_scene->onComponentAdded<Type>(*this, comp);
			return comp;
		}

		template<typename Type>
		Type &getComponent()
		{
			TST_ASSERT_MSG(hasComponent<Type>(), "Entity doesn't have component!");
			return m_scene->m_registry.get<Type>(m_handle);
		}

		template<typename Type>
		const Type &getComponent() const
		{
			TST_ASSERT_MSG(hasComponent<Type>(), "Entity doesn't have component!");
			return m_scene->m_registry.get<Type>(m_handle);
		}

		// returns nullptr if entity does not have the requested component type
		template<typename Type>
		Type *tryGetComponent()
		{
			return m_scene->m_registry.try_get<Type>(m_handle);
		}

		// returns nullptr if entity does not have the requested component type
		template<typename Type>
		const Type *tryGetComponent() const
		{
			return m_scene->m_registry.try_get<Type>(m_handle);
		}

		template<typename... Type>
		bool hasComponent()
		{
			return m_scene->m_registry.all_of<Type...>(m_handle);
		}

		template<typename... Type>
		[[nodiscard]] bool hasComponent() const
		{
			return m_scene->m_registry.all_of<Type...>(m_handle);
		}

		template<typename... Type>
		bool hasAny()
		{
			return m_scene->m_registry.any_of<Type...>(m_handle);
		}

		template<typename... Type>
		[[nodiscard]] bool hasAny() const
		{
			return m_scene->m_registry.any_of<Type...>(m_handle);
		}

		template<typename Type>
		void removeComponent()
		{
			TST_ASSERT_MSG(hasComponent<Type>(), "Entity doesn't have component!");
			m_scene->m_registry.remove<Type>(m_handle);
		}

		template<typename Type>
		void removeComponentIfExists()
		{
			if (hasComponent<Type>())
				m_scene->m_registry.remove<Type>(m_handle);
		}

		operator bool() const { return (m_handle != entt::null) && m_scene && m_scene->m_registry.valid(m_handle); }
		operator uint32() const { return static_cast<uint32>(m_handle); }
		operator entt::entity() const { return m_handle; }

		bool operator==(Entity p_entity) const { return m_handle == p_entity.m_handle; }

	private:
		entt::entity m_handle{entt::null};
		Scene *      m_scene{nullptr};
	};
}
