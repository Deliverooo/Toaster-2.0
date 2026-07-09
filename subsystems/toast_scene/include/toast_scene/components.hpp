#pragma once

#include "scene_camera.hpp"

#include "toast_lib/uuid.hpp"
#include "toast_render/mesh.hpp"
#include "toast_render/dynamic_mesh.hpp"
#include "toast_script/script_object.hpp"

#define DEFINE_COMPONENT(__name) struct TST_SCENE_API __name

namespace toaster::scene
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

		TagComponent(const String &p_tag) : tag(p_tag)
		{
		}

		auto reset() -> void { tag.clear(); }

		String tag;
	};

	DEFINE_COMPONENT(TransformComponent)
	{
		TransformComponent()  = default;
		~TransformComponent() = default;

		[[nodiscard]] auto XM_CALLCONV getTransform() const -> Dx::XMMATRIX
		{
			Dx::XMVECTOR simd_orientation{Dx::XMLoadFloat4(&orientation)};
			Dx::XMVECTOR simd_translation{Dx::XMLoadFloat3(&translation)};
			Dx::XMVECTOR simd_scale{Dx::XMLoadFloat3(&scale)};

			Dx::XMMATRIX transformation{
				Dx::XMMatrixTransformation(Dx::XMVectorZero(), Dx::XMVectorZero(), simd_scale, Dx::XMVectorZero(), simd_orientation, simd_translation)
			};
			return transformation;
		}

		auto XM_CALLCONV setTransform(Dx::FXMMATRIX p_transform) -> void
		{
			Dx::XMVECTOR out_scale;
			Dx::XMVECTOR out_orientation;
			Dx::XMVECTOR out_translation;
			Dx::XMMatrixDecompose(&out_scale, &out_orientation, &out_translation, p_transform);

			Dx::XMStoreFloat4(&orientation, out_orientation);
			Dx::XMStoreFloat3(&translation, out_translation);
			Dx::XMStoreFloat3(&scale, out_scale);
		}

		auto reset() -> void
		{
			orientation = {0.0f, 0.0f, 0.0f, 1.0f};
			translation = {0.0f, 0.0f, 0.0f};
			scale       = {1.0f, 1.0f, 1.0f};
		}

		// For alignment... :)
		Dx::XMFLOAT4 orientation{0.0f, 0.0f, 0.0f, 1.0f};
		Dx::XMFLOAT3 translation{0.0f, 0.0f, 0.0f};
		Dx::XMFLOAT3 scale{1.0f, 1.0f, 1.0f};
	};

	DEFINE_COMPONENT(SpriteRendererComponent)
	{
		SpriteRendererComponent()  = default;
		~SpriteRendererComponent() = default;

		auto reset() -> void
		{
			colour = tsm::float4{1.0f};
			texture.reset();
			tilingFactor = 1.0f;
		}

		tsm::float4          colour{1.0f};
		gpu::Texture2DHandle texture{nullptr};
		float32              tilingFactor{1.0f};
	};

	DEFINE_COMPONENT(MeshComponent)
	{
		MeshComponent()  = default;
		~MeshComponent() = default;

		auto reset() -> void { mesh.reset(); }

		render::MeshHandle mesh{nullptr};
	};

	DEFINE_COMPONENT(OldDynamicMeshComponent)
	{
		OldDynamicMeshComponent()  = default;
		~OldDynamicMeshComponent() = default;

		auto reset() -> void { mesh.reset(); }

		render::DynamicMeshOLDHandle mesh{nullptr};
	};

	DEFINE_COMPONENT(DynamicMeshComponent)
	{
		DynamicMeshComponent()  = default;
		~DynamicMeshComponent() = default;

		auto reset() -> void { mesh.reset(); }

		render::DynamicMeshHandle mesh{nullptr};
	};

	DEFINE_COMPONENT(SubmeshComponent)
	{
		SubmeshComponent()  = default;
		~SubmeshComponent() = default;

		auto reset() -> void { mesh.reset(nullptr); }

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
		tsm::float3 radiance{1.0f, 1.0f, 1.0f};
		float32     multiplier{1.0f};

		auto reset() -> void
		{
			radiance   = {1.0f, 1.0f, 1.0f};
			multiplier = 1.0f;
		}
	};

	DEFINE_COMPONENT(PointLightComponent)
	{
		tsm::float3 radiance{1.0f, 1.0f, 1.0f};
		float32     multiplier{1.0f};

		auto reset() -> void
		{
			radiance   = {1.0f, 1.0f, 1.0f};
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
