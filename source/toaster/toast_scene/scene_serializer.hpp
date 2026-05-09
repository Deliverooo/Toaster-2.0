#pragma once

#include "scene.hpp"

namespace YAML
{
	class Emitter;
	class Node;
}

namespace toaster
{
	class TST_API SceneSerializer
	{
	public:
		SceneSerializer(const RefPtr<Scene> &p_scene, const io::filesystem::Path &p_binary_dir);

		auto serialize(const io::filesystem::Path &p_filepath) -> void;
		auto serializeToYAML(YAML::Emitter &p_out) -> void;

		auto deserialize(const io::filesystem::Path &p_filepath) -> bool;
		auto deserializeFromYAML(const String &p_yaml_string) -> bool;

	private:
		auto _serializeEntity(YAML::Emitter &p_out, Entity p_entity, const RefPtr<Scene> &p_scene) -> void;
		auto _deserializeEntities(YAML::Node &p_entities, const RefPtr<Scene> &p_scene) -> void;

		RefPtr<Scene>        m_scene;
		io::filesystem::Path m_binaryDir;
	};
}
