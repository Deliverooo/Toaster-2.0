#pragma once

#include "toast_lib/string.hpp"

#include <utility>

#include "scene_camera.hpp"
#include "scriptable_entity.hpp"
#include "glm/gtx/quaternion.hpp"

#include "toast_gpu/vk/vk_texture.hpp"
#include "toast_lib/uuid.hpp"
#include "toast_scripting/script_common.hpp"
#include "toast_scripting/script_object.hpp"

#define DEFINE_COMPONENT(__name) struct TST_API __name

namespace toaster
{
	DEFINE_COMPONENT(UUIDComponent)
	{
		UUIDComponent()  = default;
		~UUIDComponent() = default;

		UUIDComponent(const UUID p_uuid) : uuid(p_uuid)
		{
		}

		UUID uuid;
	};

	DEFINE_COMPONENT(TagComponent)
	{
		TagComponent()  = default;
		~TagComponent() = default;

		TagComponent(String p_tag) : tag(std::move(p_tag))
		{
		}

		auto reset() -> void
		{
			tag.clear();
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

		[[nodiscard]] auto getTransform() const -> glm::mat4
		{
			return glm::translate(glm::mat4{1.0f}, translation) * glm::toMat4(rotation) * glm::scale(glm::mat4{1.0f}, scale);
		}

		auto reset() -> void
		{
			translation = glm::vec3{0.0f, 0.0f, 0.0f};
			rotation    = glm::quat{1.0f, 0.0f, 0.0f, 0.0f};
			scale       = glm::vec3{1.0f, 1.0f, 1.0f};
		}

		glm::vec3 translation{0.0f};
		glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f};
		glm::vec3 scale{1.0f};
	};

	DEFINE_COMPONENT(SpriteRendererComponent)
	{
		SpriteRendererComponent()  = default;
		~SpriteRendererComponent() = default;

		auto reset() -> void
		{
			colour       = glm::vec4{1.0f};
			texture      = nullptr;
			tilingFactor = 1.0f;
		}

		glm::vec4                colour{1.0f};
		RefPtr<gpu::VKTexture2D> texture{nullptr};
		float32                  tilingFactor{1.0f};
	};

	DEFINE_COMPONENT(MeshComponent)
	{
		MeshComponent()  = default;
		~MeshComponent() = default;

		auto reset() -> void
		{
			mesh.reset(nullptr);
		}

		RefPtr<gpu::VKMesh> mesh{nullptr};
	};

	DEFINE_COMPONENT(CameraComponent)
	{
		CameraComponent()  = default;
		~CameraComponent() = default;

		auto reset() -> void
		{
			camera         = SceneCamera{};
			projectionType = SceneCamera::EProjectionType::eOrthographic;
			primary        = false;
		}

		SceneCamera                  camera;
		SceneCamera::EProjectionType projectionType{SceneCamera::EProjectionType::eOrthographic};
		bool                         primary{false};
	};

	DEFINE_COMPONENT(DirectionalLightComponent)
	{
		glm::vec3 radiance{1.0f};
		float32   multiplier{1.0f};

		auto reset() -> void
		{
			radiance   = glm::vec3{1.0f};
			multiplier = 1.0f;
		}
	};

	DEFINE_COMPONENT(PointLightComponent)
	{
		glm::vec3 radiance{1.0f};
		float32   multiplier{1.0f};

		auto reset() -> void
		{
			radiance   = glm::vec3{1.0f};
			multiplier = 1.0f;
		}
	};

	DEFINE_COMPONENT(SpotLightComponent)
	{
		glm::vec3 radiance{1.0f};
		float32   falloff{1.0f};
		float32   multiplier{1.0f};
		float32   angle{67.0f};
		float32   range{12.0f};

		auto reset() -> void
		{
			radiance   = glm::vec3{1.0f};
			falloff    = 1.0f;
			multiplier = 1.0f;
			angle      = 67.0f;
			range      = 12.0f;
		}
	};

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

	DEFINE_COMPONENT(ScriptComponent)
	{
		String className{};
	};
}
