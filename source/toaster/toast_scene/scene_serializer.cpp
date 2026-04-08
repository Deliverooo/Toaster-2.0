#include "scene_serializer.hpp"
#include "entity.hpp"

#include "components.hpp"

#include "toast_lib/logging.hpp"
#include "toast_lib/toast_assert.h"

#include "toast_lib/io/filesystem.hpp"

#include <fstream>
#include <yaml-cpp/yaml.h>

#include "toast_gpu/vk/vk_texture.hpp"

inline YAML::Emitter &operator<<(YAML::Emitter &out, const glm::vec2 &v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
	return out;
}

inline YAML::Emitter &operator<<(YAML::Emitter &out, const glm::vec3 &v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
	return out;
}

inline YAML::Emitter &operator<<(YAML::Emitter &out, const glm::vec4 &v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
	return out;
}

inline YAML::Emitter &operator<<(YAML::Emitter &out, const glm::quat &v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.w << v.x << v.y << v.z << YAML::EndSeq;
	return out;
}

namespace YAML
{
	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2 &rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			return node;
		}

		static bool decode(const Node &node, glm::vec2 &rhs)
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
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3 &rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node &node, glm::vec3 &rhs)
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
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4 &rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node &node, glm::vec4 &rhs)
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
	struct convert<glm::quat>
	{
		static Node encode(const glm::quat &rhs)
		{
			Node node;
			node.push_back(rhs.w);
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node &node, glm::quat &rhs)
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

namespace toaster
{
	SceneSerializer::SceneSerializer(const RefPtr<Scene> &p_scene) : m_scene(p_scene)
	{
		TST_ASSERT_MSG(p_scene, "Scene is null");
	}

	void SceneSerializer::serialize(const io::filesystem::Path &p_filepath)
	{
		YAML::Emitter out;
		serializeToYAML(out);

		io::filesystem::writeFile(p_filepath, out.c_str());
	}

	void SceneSerializer::serializeToYAML(YAML::Emitter &p_out)
	{
		p_out << YAML::BeginMap;
		p_out << YAML::Key << "Scene" << YAML::Value << m_scene->getName();
		p_out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

		for (auto entity: m_scene->getRegistry().view<entt::entity>())
		{
			Entity e = {entity, m_scene.get()};
			if (!e)
				continue;

			serializeEntity(p_out, e, m_scene);
		}

		p_out << YAML::EndSeq;
		p_out << YAML::EndMap;
	}

	bool SceneSerializer::deserialize(const io::filesystem::Path &p_filepath)
	{
		const String yaml_string = io::filesystem::readFile(p_filepath);

		try
		{
			TST_ASSERT(deserializeFromYAML(yaml_string));
		}
		catch (const YAML::Exception &e)
		{
			LOG_ERROR("Failed to deserialize scene '{0}': {1}", p_filepath.string(), e.what());
			return false;
		}

		return true;
	}

	bool SceneSerializer::deserializeFromYAML(const String &p_yaml_string)
	{
		YAML::Node data = YAML::Load(p_yaml_string);

		const auto scene_name = data["Scene"];
		if (!scene_name)
			return false;

		m_scene->setName(scene_name.as<String>());

		auto entities = data["Entities"];
		if (entities)
			deserializeEntities(entities, m_scene);

		return true;
	}

	void SceneSerializer::serializeEntity(YAML::Emitter &p_out, Entity p_entity, const RefPtr<Scene> &p_scene)
	{
		p_out << YAML::BeginMap;
		p_out << YAML::Key << "Entity" << YAML::Value << "67676767";

		{
			TST_ASSERT_MSG(p_entity.hasComponent<TagComponent>(), "Tag component is null");
			p_out << YAML::Key << "TagComponent" << YAML::BeginMap;
			const auto &tag = p_entity.getComponent<TagComponent>().tag;
			p_out << YAML::Key << "Tag" << YAML::Value << tag;
			p_out << YAML::EndMap;
		}

		{
			TST_ASSERT_MSG(p_entity.hasComponent<TransformComponent>(), "Transform component is null");
			p_out << YAML::Key << "TransformComponent" << YAML::BeginMap;

			const auto &transform = p_entity.getComponent<TransformComponent>();
			p_out << YAML::Key << "Translation" << YAML::Value << transform.translation;
			p_out << YAML::Key << "Rotation" << YAML::Value << transform.rotation;
			p_out << YAML::Key << "Scale" << YAML::Value << transform.scale;

			p_out << YAML::EndMap;
		}

		if (p_entity.hasComponent<SpriteRendererComponent>())
		{
			p_out << YAML::Key << "SpriteRendererComponent" << YAML::BeginMap;

			const auto &src = p_entity.getComponent<SpriteRendererComponent>();
			p_out << YAML::Key << "Colour" << YAML::Value << src.colour;

			auto texture_path = src.texture ? src.texture->getPath() : "Null";
			p_out << YAML::Key << "TexturePath" << YAML::Value << texture_path.string();
			p_out << YAML::Key << "TilingFactor" << YAML::Value << src.tilingFactor;

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

		p_out << YAML::EndMap;
	}

	void SceneSerializer::deserializeEntities(YAML::Node &p_entities, const RefPtr<Scene> &p_scene)
	{
		for (auto entity: p_entities)
		{
			uint64 uuid = entity["Entity"].as<uint64>();

			String entity_name;
			auto   tag_comp = entity["TagComponent"];
			if (tag_comp)
				entity_name = tag_comp["Tag"].as<String>();

			Entity out_entity = p_scene->createEntity(entity_name);

			auto transform_comp = entity["TransformComponent"];
			if (transform_comp)
			{
				auto &tc       = out_entity.getComponent<TransformComponent>();
				tc.translation = transform_comp["Translation"].as<glm::vec3>();
				tc.rotation    = transform_comp["Rotation"].as<glm::vec3>();
				tc.scale       = transform_comp["Scale"].as<glm::vec3>();
			}

			auto sprite_comp = entity["SpriteRendererComponent"];
			if (sprite_comp)
			{
				auto &src  = out_entity.addComponent<SpriteRendererComponent>();
				src.colour = sprite_comp["Colour"].as<glm::vec4>();

				auto texture_path = sprite_comp["TexturePath"].as<String>();
				if (texture_path != "Null")
				{
					gpu::TextureSpecInfo texture_spec{};
					texture_spec.generateMips = false;
					src.texture               = make_reference<gpu::VKTexture2D>(p_scene->m_ctx, texture_spec, texture_path);
				}
				else
					src.texture = nullptr;

				LOG_TRACE("Sprite texture: {}", texture_path);

				src.tilingFactor = sprite_comp["TilingFactor"].as<float32>();
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

				cc.projectionType = static_cast<SceneCamera::EProjectionType>(camera_node["ProjectionType"].as<int32>());
				cc.primary        = camera_comp["Primary"].as<bool>();
			}
		}
	}
}
