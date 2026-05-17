#pragma once

#include "scene.hpp"

namespace toaster
{
	class TST_API SceneImporter
	{
	public:
		SceneImporter(Scene *p_scene);
		~SceneImporter();

		auto importFromFile(const io::filesystem::Path &p_path) -> Entity; // Returns the root entity of the scene

	private:
		NonOwningPtr<Scene> m_scene{nullptr};

		void* m_importer{nullptr}; // Actually an Assimp::Importer*
	};
}
