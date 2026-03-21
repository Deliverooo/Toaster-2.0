#pragma once

#include "toaster/toast_lib/string.hpp"

#include <utility>

#include "scene_camera.hpp"
#include "scriptable_entity.hpp"

#include "toaster/toast_gpu/texture.hpp"

#define DEFINE_COMPONENT(__name) struct __name

namespace toaster
{
	DEFINE_COMPONENT(TagComponent)
	{
		TagComponent()  = default;
		~TagComponent() = default;

		TagComponent(U8String p_tag) : tag(std::move(p_tag))
		{
		}

		U8String tag;
	};

	DEFINE_COMPONENT(TransformComponent)
	{
		TransformComponent()  = default;
		~TransformComponent() = default;

		TransformComponent(const glm::mat4 &p_transform) : transform(p_transform)
		{
		}

		glm::mat4 transform{1.0f};
	};

	DEFINE_COMPONENT(SpriteRendererComponent)
	{
		glm::vec4               colour{1.0f};
		RefPtr<gpu::ITexture2D> texture{nullptr};
	};

	DEFINE_COMPONENT(CameraComponent)
	{
		CameraComponent()  = default;
		~CameraComponent() = default;

		enum class EProjectionType
		{
			ePerspective, eOrthographic
		};

		SceneCamera     camera;
		EProjectionType projectionType{EProjectionType::eOrthographic};
		bool            primary{true};
	};

	DEFINE_COMPONENT(NativeScriptComponent)
	{
		ScriptableEntity *instance{nullptr};

		using InstantiateFn = ScriptableEntity *(*)();
		InstantiateFn instantiateFn{nullptr};

		using DestroyFn = void(*)(NativeScriptComponent *);
		DestroyFn destroyFn{nullptr};

		template<c_ScriptableEntity Type>
		void bind()
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
