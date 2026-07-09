#pragma once

#include "components.hpp"
#include "entity.hpp"

namespace toaster::scene
{
	class InputContext;

	class TST_SCENE_API ScriptableEntity
	{
	public:
		ScriptableEntity()          = default;
		virtual ~ScriptableEntity() = default;

		virtual auto onCreate(void *p_user_data) -> void = 0;
		virtual auto onUpdate(float32 p_dt) -> void = 0;

		virtual auto onDestroy() -> void
		{
		}

		virtual auto onEvent([[maybe_unused]] Event &p_event) -> void
		{
		}

		virtual auto onResize([[maybe_unused]] tsm::uint2 p_size) -> void
		{
		}

		template<typename Type>
		auto addComponent() -> Type &
		{
			return m_entity.addComponent<Type>();
		}

		template<typename Type>
		auto getComponent() -> Type &
		{
			return m_entity.getComponent<Type>();
		}

		template<typename Type>
		auto getComponent() const -> const Type &
		{
			return m_entity.getComponent<Type>();
		}

		template<typename Type>
		auto getOrAddComponent() -> Type &
		{
			auto comp{m_entity.tryGetComponent<Type>()};
			if (comp)
				return *comp;
			return m_entity.addComponent<Type>();
		}

		template<typename Type>
		auto removeComponent() -> void
		{
			static_assert(!std::same_as<Type, TransformComponent> && !std::same_as<Type, TagComponent> && !std::same_as<Type, UUIDComponent> && !std::same_as<Type,
							  RelationshipComponent>);
			m_entity.removeComponent<Type>();
		}

		// Better than the derived classes having to get it manually
		TagComponent *const       tag{nullptr};
		TransformComponent *const transform{nullptr};

		const NonOwningPtr<Scene> scene{nullptr};

	private:
		auto _superInit() -> void
		{
			// Ts looks very unsafe
			*const_cast<TagComponent **>(&tag)             = &m_entity.getComponent<TagComponent>();
			*const_cast<TransformComponent **>(&transform) = &m_entity.getComponent<TransformComponent>();
			*const_cast<Scene **>(&scene)                  = m_entity.getScene();
		}

		Entity m_entity;
		friend class Scene;
	};

	template<typename Type> concept c_ScriptableEntity = std::derived_from<Type, ScriptableEntity> && std::default_initializable<Type>;

	DEFINE_COMPONENT(NativeScriptComponent)
	{
		ScriptableEntity *instance{nullptr};

		using InstantiateFn = ScriptableEntity *(*)();
		InstantiateFn instantiateFn{nullptr};

		using DestroyFn = void(*)(NativeScriptComponent *);
		DestroyFn destroyFn{nullptr};

		template<c_ScriptableEntity Type>
		auto bind(void *p_user_data = nullptr) -> void
		{
			instantiateFn = +[]() -> ScriptableEntity * { return static_cast<ScriptableEntity *>(new Type()); };
			destroyFn     = +[](NativeScriptComponent *p_ncs) -> void
			{
				delete p_ncs->instance;
				p_ncs->instance = nullptr;
			};

			userData = p_user_data;
		}

	private:
		void *userData{nullptr};

		friend class Scene;
	};
}
