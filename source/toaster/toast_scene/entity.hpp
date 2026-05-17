#pragma once

#include <entt/entt.hpp>
#include "scene.hpp"

#include "components.hpp"

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

		auto getUUID() const -> UUID { return getComponent<UUIDComponent>().uuid; }
		auto getTag() const -> const String & { return getComponent<TagComponent>().tag; }

		auto getTransform() const -> glm::mat4 { return getComponent<TransformComponent>().getTransform(); }
		auto getTranslation() const -> const glm::vec3 & { return getComponent<TransformComponent>().translation; }
		auto getOrientation() const -> const glm::quat & { return getComponent<TransformComponent>().rotation; }
		auto getScale() const -> const glm::vec3 & { return getComponent<TransformComponent>().scale; }

		auto getParent() const -> Entity { return m_scene->getEntityByUUID(getParentUUID()); }
		auto getParentUUID() const -> UUID { return getComponent<RelationshipComponent>().parentUUID; }

		auto setParent(Entity p_parent) -> void
		{
			auto current_parent{getParent()};
			if (current_parent == p_parent)
				return;

			if (current_parent)
				current_parent.removeChild(*this);

			setParentUUID(p_parent.getUUID());
			if (p_parent)
			{
				auto &parent_children{p_parent.getChildren()};
				if (std::ranges::find(parent_children, getUUID()) == parent_children.end())
					parent_children.emplace_back(getUUID());
			}
		}

		auto setParentUUID(UUID p_parent_uuid) -> void { getComponent<RelationshipComponent>().parentUUID = p_parent_uuid; }

		auto getChildren() const -> const std::vector<UUID> & { return getComponent<RelationshipComponent>().children; }
		auto getChildren() -> std::vector<UUID> & { return getComponent<RelationshipComponent>().children; }

		auto removeChild(Entity p_child) -> bool
		{
			const auto child_uuid{p_child.getUUID()};
			auto &     children{getChildren()};
			if (const auto it{std::ranges::find(children, child_uuid)}; it != children.end())
			{
				children.erase(it);
				return true;
			}
			return false;
		}

	private:
		entt::entity m_handle{entt::null};
		Scene *      m_scene{nullptr};
	};
}
