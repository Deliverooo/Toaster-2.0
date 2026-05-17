#pragma once

#include <utility>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

#include "scene_camera.hpp"

#include "toast_gpu/vk/vk_texture.hpp"
#include "toast_lib/uuid.hpp"
#include "toast_render/mesh.hpp"
#include "toast_scripting/script_object.hpp"

#define DEFINE_COMPONENT(__name) struct TST_API __name
#define REGISTER_COMPONENT_HASH(__name) template<>\
		struct entt::type_hash<toaster::__name> final {\
			[[nodiscard]] static consteval entt::id_type value() noexcept {\
				return entt::hashed_string::value(#__name);\
			}\
		};

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

	DEFINE_COMPONENT(RelationshipComponent)
	{
		RelationshipComponent()  = default;
		~RelationshipComponent() = default;

		RelationshipComponent(const UUID p_parent_uuid) : parentUUID(p_parent_uuid)
		{
		}

		UUID              parentUUID{UINT64_MAX}; // Invalid or doesn't have parent
		std::vector<UUID> children;
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
			return glm::translate(glm::mat4{1.0f}, translation) * glm::toMat4(orientation) * glm::scale(glm::mat4{1.0f}, scale);
		}

		auto setTransform(const glm::mat4 &p_transform) -> void
		{
			glm::vec3 skew{0.0f};
			glm::vec4 perspective{0.0f};
			glm::decompose(p_transform, scale, orientation, translation, skew, perspective);
			// tsm::decomposeTransform(p_transform, translation, rotation, scale);
		}

		auto reset() -> void
		{
			translation = glm::vec3{0.0f, 0.0f, 0.0f};
			orientation = glm::quat{1.0f, 0.0f, 0.0f, 0.0f};
			scale       = glm::vec3{1.0f, 1.0f, 1.0f};
		}

		glm::vec3 translation{0.0f};
		glm::quat orientation{1.0f, 0.0f, 0.0f, 0.0f};
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

		render::MeshHandle mesh{nullptr};
	};

	DEFINE_COMPONENT(SubmeshComponent)
	{
		SubmeshComponent()  = default;
		~SubmeshComponent() = default;

		auto reset() -> void
		{
			mesh.reset(nullptr);
		}

		render::MeshHandle mesh{nullptr};
		uint32             submeshIndex{0u};
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

	DEFINE_COMPONENT(ScriptComponent)
	{
		String className{};
	};
}

REGISTER_COMPONENT_HASH(UUIDComponent)

REGISTER_COMPONENT_HASH(TransformComponent)

REGISTER_COMPONENT_HASH(SpriteRendererComponent)

REGISTER_COMPONENT_HASH(CameraComponent)
