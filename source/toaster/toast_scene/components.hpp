#pragma once

#include "toast_lib/string.hpp"

#include <utility>

#include "scene_camera.hpp"
#include "scriptable_entity.hpp"
#include "glm/gtx/quaternion.hpp"

#include "toast_gpu/texture.hpp"

#define DEFINE_COMPONENT(__name) struct __name

namespace toaster
{
	DEFINE_COMPONENT(TagComponent)
	{
		TagComponent()  = default;
		~TagComponent() = default;

		TagComponent(String p_tag) : tag(std::move(p_tag))
		{
		}

		String tag;
	};

	DEFINE_COMPONENT(TransformComponent)
	{
		TransformComponent()  = default;
		~TransformComponent() = default;

		TransformComponent(const glm::vec3 &p_translation) : translation(p_translation)
		{
		}

		[[nodiscard]] glm::mat4 getTransform() const
		{
			return glm::translate(glm::mat4{1.0f}, translation) * glm::toMat4(glm::quat{rotation}) * glm::scale(glm::mat4{1.0f}, scale);
		}

		void reset()
		{
			translation = glm::vec3{0.0f, 0.0f, 0.0f};
			rotation    = glm::vec3{0.0f, 0.0f, 0.0f};
			scale       = glm::vec3{1.0f, 1.0f, 1.0f};
		}

		glm::vec3 translation{0.0f};
		glm::vec3 rotation{0.0f};
		glm::vec3 scale{1.0f};
	};

	DEFINE_COMPONENT(SpriteRendererComponent)
	{
		SpriteRendererComponent()  = default;
		~SpriteRendererComponent() = default;

		void reset()
		{
			colour       = glm::vec4{1.0f};
			texture      = nullptr;
			tilingFactor = 1.0f;
		}

		glm::vec4               colour{1.0f};
		RefPtr<gpu::ITexture2D> texture{nullptr};
		float32                 tilingFactor{1.0f};
	};

	DEFINE_COMPONENT(CameraComponent)
	{
		CameraComponent()  = default;
		~CameraComponent() = default;

		void reset()
		{
			camera         = SceneCamera{};
			projectionType = SceneCamera::EProjectionType::eOrthographic;
			primary        = false;
		}

		SceneCamera                  camera;
		SceneCamera::EProjectionType projectionType{SceneCamera::EProjectionType::eOrthographic};
		bool                         primary{false};
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
