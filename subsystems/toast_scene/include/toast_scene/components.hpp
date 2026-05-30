#pragma once

#include <utility>

#include "scene_camera.hpp"

#include "toast_lib/uuid.hpp"
#include "toast_render/mesh.hpp"
#include "toast_script/script_object.hpp"

#define DEFINE_COMPONENT(__name) struct TST_SCENE_API __name
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

		TransformComponent(const tsm::float3 &p_translation) : translation(p_translation)
		{
		}

		[[nodiscard]] auto getTransform() const -> tsm::float4x4
		{
			return tsm::translate(tsm::float4x4{1.0f}, translation) * tsm::toMat4(orientation) * tsm::scale(tsm::float4x4{1.0f}, scale);
		}

		auto setTransform(const tsm::float4x4 &p_transform) -> void
		{
			tsm::decomposeTransform(p_transform, translation, orientation, scale);
		}

		auto reset() -> void
		{
			translation = tsm::float3{0.0f, 0.0f, 0.0f};
			orientation = tsm::quatf{1.0f, 0.0f, 0.0f, 0.0f};
			scale       = tsm::float3{1.0f, 1.0f, 1.0f};
		}

		tsm::float3 translation{0.0f};
		tsm::quatf  orientation{1.0f, 0.0f, 0.0f, 0.0f};
		tsm::float3 scale{1.0f};
	};

	DEFINE_COMPONENT(SpriteRendererComponent)
	{
		SpriteRendererComponent()  = default;
		~SpriteRendererComponent() = default;

		auto reset() -> void
		{
			colour = tsm::float4{1.0f};
			textureAssetID.reset();
			tilingFactor = 1.0f;
		}

		tsm::float4          colour{1.0f};
		gpu::Texture2DHandle textureAssetID{nullptr};
		float32              tilingFactor{1.0f};
	};

	DEFINE_COMPONENT(MeshComponent)
	{
		MeshComponent()  = default;
		~MeshComponent() = default;

		auto reset() -> void
		{
			meshAssetID.reset();
		}

		render::MeshHandle meshAssetID{nullptr};
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
			camera  = SceneCamera{};
			primary = false;
		}

		SceneCamera camera;
		bool        primary{false};
	};

	DEFINE_COMPONENT(DirectionalLightComponent)
	{
		tsm::float3 radiance{1.0f};
		float32     multiplier{1.0f};

		auto reset() -> void
		{
			radiance   = tsm::float3{1.0f};
			multiplier = 1.0f;
		}
	};

	DEFINE_COMPONENT(PointLightComponent)
	{
		tsm::float3 radiance{1.0f};
		float32     multiplier{1.0f};

		auto reset() -> void
		{
			radiance   = tsm::float3{1.0f};
			multiplier = 1.0f;
		}
	};

	DEFINE_COMPONENT(SpotLightComponent)
	{
		tsm::float3 radiance{1.0f};
		float32     falloff{1.0f};
		float32     multiplier{1.0f};
		float32     angle{67.0f};
		float32     range{12.0f};

		auto reset() -> void
		{
			radiance   = tsm::float3{1.0f};
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
