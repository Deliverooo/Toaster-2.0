#pragma once

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
}
