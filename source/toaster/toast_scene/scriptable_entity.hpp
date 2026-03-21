#pragma once

#include "entity.hpp"

namespace toaster
{
	class ScriptableEntity
	{
	public:
		ScriptableEntity()          = default;
		virtual ~ScriptableEntity() = default;

		virtual void onCreate() = 0;
		virtual void onUpdate(float32 p_dt) = 0;
		virtual void onDestroy() = 0;

		template<typename Type>
		Type &getComponent()
		{
			return m_entity.getComponent<Type>();
		}

		template<typename Type>
		const Type &getComponent() const
		{
			return m_entity.getComponent<Type>();
		}

	private:
		Entity m_entity;
		friend class Scene;
	};

	template<typename Type> concept c_ScriptableEntity = std::derived_from<Type, ScriptableEntity> && std::default_initializable<Type>;
}
