#pragma once

#include "components.hpp"
#include "entity.hpp"

namespace toaster
{
	class TST_API ScriptableEntity
	{
	public:
		ScriptableEntity()          = default;
		virtual ~ScriptableEntity() = default;

		virtual auto onCreate() -> void = 0;
		virtual auto onUpdate(float32 p_dt) -> void = 0;
		virtual auto onDestroy() -> void = 0;

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

	private:
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
		auto bind() -> void
		{
			instantiateFn = []() -> ScriptableEntity * { return static_cast<ScriptableEntity *>(new Type()); };
			destroyFn     = [](NativeScriptComponent *p_ncs) -> void
			{
				delete p_ncs->instance;
				p_ncs->instance = nullptr;
			};
		}
	};
}
