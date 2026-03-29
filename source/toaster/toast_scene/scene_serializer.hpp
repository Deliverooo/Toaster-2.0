#pragma once

#include "scene.hpp"

namespace YAML
{
	class Emitter;
	class Node;
}

namespace toaster
{
	class SceneSerializer
	{
	public:
		SceneSerializer(const RefPtr<Scene> &p_scene);

		void serialize(const io::filesystem::Path &p_filepath);
		void serializeToYAML(YAML::Emitter &p_out);

		bool deserialize(const io::filesystem::Path &p_filepath);
		bool deserializeFromYAML(const String &p_yaml_string);

		static void serializeEntity(YAML::Emitter &p_out, Entity p_entity, const RefPtr<Scene>& p_scene);
		static void deserializeEntities(YAML::Node &p_entities, const RefPtr<Scene>& p_scene);

	private:
		RefPtr<Scene> m_scene;
	};
}
