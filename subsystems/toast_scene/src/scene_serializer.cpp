#include "toast_scene/scene_serializer.hpp"
#include "toast_scene/entity.hpp"

#include "toast_render/render_context.hpp"

#include <yaml-cpp/yaml.h>

inline auto operator<<(YAML::Emitter &out, const tsm::float2 &v) -> YAML::Emitter &
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
	return out;
}

inline auto operator<<(YAML::Emitter &out, const tsm::float3 &v) -> YAML::Emitter &
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
	return out;
}

inline auto operator<<(YAML::Emitter &out, const tsm::float4 &v) -> YAML::Emitter &
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
	return out;
}

inline auto operator<<(YAML::Emitter &out, const Dx::XMFLOAT2 &v) -> YAML::Emitter &
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
	return out;
}

inline auto operator<<(YAML::Emitter &out, const Dx::XMFLOAT3 &v) -> YAML::Emitter &
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
	return out;
}

inline auto operator<<(YAML::Emitter &out, const Dx::XMFLOAT4 &v) -> YAML::Emitter &
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
	return out;
}

inline auto operator<<(YAML::Emitter &out, const tsm::quatf &v) -> YAML::Emitter &
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.w << v.x << v.y << v.z << YAML::EndSeq;
	return out;
}

namespace YAML
{
	template<>
	struct convert<tsm::float2>
	{
		static auto encode(const tsm::float2 &rhs) -> Node
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			return node;
		}

		static auto decode(const Node &node, tsm::float2 &rhs) -> bool
		{
			if (!node.IsSequence() || node.size() != 2)
			{
				return false;
			}

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<tsm::float3>
	{
		static auto encode(const tsm::float3 &rhs) -> Node
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static auto decode(const Node &node, tsm::float3 &rhs) -> bool
		{
			if (!node.IsSequence() || node.size() != 3)
			{
				return false;
			}

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<tsm::float4>
	{
		static auto encode(const tsm::float4 &rhs) -> Node
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static auto decode(const Node &node, tsm::float4 &rhs) -> bool
		{
			if (!node.IsSequence() || node.size() != 4)
			{
				return false;
			}

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};

	template<>
	struct convert<Dx::XMFLOAT2>
	{
		static auto encode(const Dx::XMFLOAT2 &rhs) -> Node
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			return node;
		}

		static auto decode(const Node &node, Dx::XMFLOAT2 &rhs) -> bool
		{
			if (!node.IsSequence() || node.size() != 2)
			{
				return false;
			}

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<Dx::XMFLOAT3>
	{
		static auto encode(const Dx::XMFLOAT3 &rhs) -> Node
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static auto decode(const Node &node, Dx::XMFLOAT3 &rhs) -> bool
		{
			if (!node.IsSequence() || node.size() != 3)
			{
				return false;
			}

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<Dx::XMFLOAT4>
	{
		static auto encode(const Dx::XMFLOAT4 &rhs) -> Node
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static auto decode(const Node &node, Dx::XMFLOAT4 &rhs) -> bool
		{
			if (!node.IsSequence() || node.size() != 4)
			{
				return false;
			}

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};

	template<>
	struct convert<tsm::quatf>
	{
		static auto encode(const tsm::quatf &rhs) -> Node
		{
			Node node;
			node.push_back(rhs.w);
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static auto decode(const Node &node, tsm::quatf &rhs) -> bool
		{
			if (!node.IsSequence() || node.size() != 4)
			{
				return false;
			}

			rhs.w = node[0].as<float>();
			rhs.x = node[1].as<float>();
			rhs.y = node[2].as<float>();
			rhs.z = node[3].as<float>();
			return true;
		}
	};
}

namespace toaster::scene
{
	SceneSerializer::SceneSerializer(const RefPtr<Scene> &p_scene) : m_scene(p_scene)
	{
		TST_PERMA_ASSERT_MSG(p_scene, "Scene is null");
	}

	auto SceneSerializer::serialize(const io::filesystem::Path &p_filepath) -> void
	{
		YAML::Emitter out;
		serializeToYAML(out);

		io::filesystem::writeFile(p_filepath, out.c_str());
	}

	auto SceneSerializer::serializeToYAML(YAML::Emitter &p_out) -> void
	{
		p_out << YAML::BeginMap;

		p_out << YAML::Key << "Scene" << YAML::BeginMap;
		p_out << YAML::Key << "Name" << YAML::Value << m_scene->getName();

		// String env_path{m_scene->m_sceneEnvironment.skyboxMap->getPath().string()};
		p_out << YAML::Key << "SceneEnvironmentAssetID" << YAML::Value << "skibidi";

		p_out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

		for (auto entity: m_scene->getRegistry().view<entt::entity>())
		{
			Entity e = {entity, m_scene.get()};
			if (!e)
				continue;

			_serializeEntity(p_out, e, m_scene);
		}

		p_out << YAML::EndSeq;
		p_out << YAML::EndMap;
		p_out << YAML::EndMap;
	}

	auto SceneSerializer::deserialize(const io::filesystem::Path &p_filepath) -> bool
	{
		const String yaml_string = io::filesystem::readFile(p_filepath);

		try
		{
			TST_PERMA_ASSERT(deserializeFromYAML(yaml_string));
		}
		catch (const YAML::Exception &e)
		{
			LOG_ERROR("Failed to deserialize scene '{0}': {1}", p_filepath.string(), e.what());
			TST_PERMA_ASSERT(false);
			return false;
		}

		return true;
	}

	auto SceneSerializer::deserializeFromYAML(const String &p_yaml_string) -> bool
	{
		YAML::Node data = YAML::Load(p_yaml_string);

		const auto scene_node = data["Scene"];
		if (!scene_node)
			return false;

		m_scene->setName(scene_node["Name"].as<String>());

		// m_scene->m_sceneEnvironment.skyboxMap = m_scene->m_renderCtx->createEnvironmentMap(scene_node["SceneEnvironmentAssetID"].as<String>());

		auto entities = scene_node["Entities"];
		if (entities)
			_deserializeEntities(entities, m_scene);

		return true;
	}

	auto SceneSerializer::_serializeEntity(YAML::Emitter &p_out, Entity p_entity, [[maybe_unused]] const RefPtr<Scene> &p_scene) -> void
	{
		p_out << YAML::BeginMap;
		p_out << YAML::Key << "Entity" << YAML::Value << p_entity.getComponent<UUIDComponent>().uuid;

		{
			TST_PERMA_ASSERT_MSG(p_entity.hasComponent<TagComponent>(), "Tag component is null");
			p_out << YAML::Key << "TagComponent" << YAML::BeginMap;
			const auto &tag = p_entity.getComponent<TagComponent>().tag;
			p_out << YAML::Key << "Tag" << YAML::Value << tag;
			p_out << YAML::EndMap;
		}

		{
			TST_PERMA_ASSERT_MSG(p_entity.hasComponent<TransformComponent>(), "Transform component is null");
			p_out << YAML::Key << "TransformComponent" << YAML::BeginMap;

			const auto &transform = p_entity.getComponent<TransformComponent>();
			p_out << YAML::Key << "Translation" << YAML::Value << transform.translation;
			p_out << YAML::Key << "Rotation" << YAML::Value << transform.orientation;
			p_out << YAML::Key << "Scale" << YAML::Value << transform.scale;

			p_out << YAML::EndMap;
		}

		if (p_entity.hasComponent<SpriteRendererComponent>())
		{
			p_out << YAML::Key << "SpriteRendererComponent" << YAML::BeginMap;

			const auto &src = p_entity.getComponent<SpriteRendererComponent>();
			p_out << YAML::Key << "Colour" << YAML::Value << src.colour;
			p_out << YAML::Key << "TextureAssetID" << YAML::Value << src.texture;
			p_out << YAML::Key << "TilingFactor" << YAML::Value << src.tilingFactor;

			p_out << YAML::EndMap;
		}

		if (p_entity.hasComponent<MeshComponent>())
		{
			const auto &mc{p_entity.getComponent<MeshComponent>()};

			p_out << YAML::Key << "MeshComponent";
			p_out << YAML::BeginMap;
			p_out << YAML::Key << "MeshAssetID" << YAML::Value << mc.mesh;
			p_out << YAML::EndMap;
		}

		if (p_entity.hasComponent<CameraComponent>())
		{
			p_out << YAML::Key << "CameraComponent";
			p_out << YAML::BeginMap;

			const auto &cameraComponent = p_entity.getComponent<CameraComponent>();
			const auto &camera          = cameraComponent.camera;

			p_out << YAML::Key << "Camera" << YAML::Value;
			p_out << YAML::BeginMap;
			p_out << YAML::Key << "ProjectionType" << YAML::Value << static_cast<int32>(camera.getProjectionType());
			p_out << YAML::Key << "PerspectiveFov" << YAML::Value << camera.getPerspectiveFov();
			p_out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.getPerspectiveNearClip();
			p_out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.getPerspectiveFarClip();
			p_out << YAML::Key << "OrthoSize" << YAML::Value << camera.getOrthoSize();
			p_out << YAML::Key << "OrthoNear" << YAML::Value << camera.getOrthoNearClip();
			p_out << YAML::Key << "OrthoFar" << YAML::Value << camera.getOrthoFarClip();
			p_out << YAML::EndMap;
			p_out << YAML::Key << "Primary" << YAML::Value << cameraComponent.primary;

			p_out << YAML::EndMap;
		}

		if (p_entity.hasComponent<DirectionalLightComponent>())
		{
			const auto &dlc{p_entity.getComponent<DirectionalLightComponent>()};

			p_out << YAML::Key << "DirectionalLightComponent";
			p_out << YAML::BeginMap;

			p_out << YAML::Key << "Radiance" << YAML::Value << dlc.radiance;
			p_out << YAML::Key << "Multiplier" << YAML::Value << dlc.multiplier;
			p_out << YAML::EndMap;
		}

		if (p_entity.hasComponent<PointLightComponent>())
		{
			const auto &plc{p_entity.getComponent<PointLightComponent>()};

			p_out << YAML::Key << "PointLightComponent";
			p_out << YAML::BeginMap;

			p_out << YAML::Key << "Radiance" << YAML::Value << plc.radiance;
			p_out << YAML::Key << "Multiplier" << YAML::Value << plc.multiplier;
			p_out << YAML::EndMap;
		}

		if (p_entity.hasComponent<ScriptComponent>())
		{
			const auto &sc{p_entity.getComponent<ScriptComponent>()};

			p_out << YAML::Key << "ScriptComponent";
			p_out << YAML::BeginMap;
			p_out << YAML::Key << "ClassName" << YAML::Value << sc.className;
			p_out << YAML::EndMap;
		}

		p_out << YAML::EndMap;
	}

	auto SceneSerializer::_deserializeEntities(YAML::Node &p_entities, const RefPtr<Scene> &p_scene) -> void
	{
		for (auto entity: p_entities)
		{
			UUID uuid{entity["Entity"].as<uint64>()};

			String entity_name;
			auto   tag_comp = entity["TagComponent"];
			if (tag_comp)
				entity_name = tag_comp["Tag"].as<String>();

			Entity out_entity = p_scene->createEntityWithUUID(uuid, entity_name);

			auto transform_comp = entity["TransformComponent"];
			if (transform_comp)
			{
				auto &tc       = out_entity.getComponent<TransformComponent>();
				tc.translation = transform_comp["Translation"].as<Dx::XMFLOAT3>();
				tc.orientation = transform_comp["Rotation"].as<Dx::XMFLOAT4>();
				tc.scale       = transform_comp["Scale"].as<Dx::XMFLOAT3>();
			}

			auto sprite_comp = entity["SpriteRendererComponent"];
			if (sprite_comp)
			{
				auto &src        = out_entity.addComponent<SpriteRendererComponent>();
				src.colour       = sprite_comp["Colour"].as<tsm::float4>();;
				src.texture      = m_scene->m_renderCtx->createGPURef<gpu::VKTexture2D>(gpu::TextureSpecInfo{}, sprite_comp["TextureAssetID"].as<String>());
				src.tilingFactor = sprite_comp["TilingFactor"].as<float32>();
			}

			auto mesh_comp{entity["MeshComponent"]};
			if (mesh_comp)
			{
				auto &mc{out_entity.addComponent<MeshComponent>()};

				String mesh_asset_path{mesh_comp["MeshAssetID"].as<String>()};
				mc.mesh = m_scene->m_renderCtx->createRef<render::MeshData>(mesh_asset_path);
			}

			auto camera_comp = entity["CameraComponent"];
			if (camera_comp)
			{
				auto  camera_node = camera_comp["Camera"];
				auto &cc          = out_entity.addComponent<CameraComponent>();
				auto &camera      = cc.camera;

				camera.setProjectionType(static_cast<SceneCamera::EProjectionType>(camera_node["ProjectionType"].as<int32>()));

				camera.setPerspectiveFov(camera_node["PerspectiveFov"].as<float32>());
				camera.setPerspectiveNearClip(camera_node["PerspectiveNear"].as<float32>());
				camera.setPerspectiveFarClip(camera_node["PerspectiveFar"].as<float32>());

				camera.setOrthoSize(camera_node["OrthoSize"].as<float32>());
				camera.setOrthoNearClip(camera_node["OrthoNear"].as<float32>());
				camera.setOrthoFarClip(camera_node["OrthoFar"].as<float32>());

				cc.primary = camera_comp["Primary"].as<bool>();
			}

			auto directional_light_comp{entity["DirectionalLightComponent"]};
			if (directional_light_comp)
			{
				auto &dlc{out_entity.addComponent<DirectionalLightComponent>()};
				dlc.radiance   = directional_light_comp["Radiance"].as<tsm::float3>();
				dlc.multiplier = directional_light_comp["Multiplier"].as<float32>();
			}

			auto point_light_comp{entity["PointLightComponent"]};
			if (point_light_comp)
			{
				auto &plc{out_entity.addComponent<PointLightComponent>()};
				plc.radiance   = point_light_comp["Radiance"].as<tsm::float3>();
				plc.multiplier = point_light_comp["Multiplier"].as<float32>();
			}

			auto script_comp{entity["ScriptComponent"]};
			if (script_comp)
			{
				auto &sc{out_entity.addComponent<ScriptComponent>()};
				sc.className = script_comp["ClassName"].as<String>();
			}
		}
	}
}
