#pragma once

#include "toaster_macros.hpp"
#include "toast_asset/asset_manager.hpp"

#include "toast_lib/ptr.hpp"
#include "toast_lib/string.hpp"
#include "toast_lib/io/filesystem.hpp"

namespace toaster
{
	struct TST_API ProjectSpecInfo
	{
		String name{"New_Project"};

		String assetRegistryPath{"resources/asset_registry.treg"};

		// These are strings because they are only the names of the directories relative to the actual .tproj file
		String scriptDirectory{"resources/scripts"};
		String sceneDirectory{"resources/scenes"};
		String meshDirectory{"resources/meshes"};
		String textureDirectory{"resources/textures"};

		String startupSceneName{"New_Scene"};
	};

	class TST_API Project
	{
	public:
		Project();
		Project(render::RenderContext *p_render_ctx);

		auto getFullResourcesDirectory() const -> io::filesystem::Path;
		auto getFullAssetRegistryPath() const -> io::filesystem::Path;
		auto getFullScriptDirectory() const -> io::filesystem::Path;
		auto getFullSceneDirectory() const -> io::filesystem::Path;
		auto getFullMeshDirectory() const -> io::filesystem::Path;
		auto getFullTextureDirectory() const -> io::filesystem::Path;

		auto getStartupScenePath() const -> io::filesystem::Path;
		auto getFullStartupScenePath() const -> io::filesystem::Path;

		auto getPath() const -> const io::filesystem::Path &;  // Returns the path to the .tproj file
		auto getRootDirectory() const -> io::filesystem::Path; // Returns the directory that the .tproj file is in
		auto getSpecInfo() const -> const ProjectSpecInfo &;

		auto getAssetManager() const -> asset::AssetManager &;

		auto printInfo() const -> void;

	private:
		// The actual path to the .tproj file on disk. All other paths are relative to this one
		io::filesystem::Path m_path{};

		ProjectSpecInfo m_specInfo{};

		UniquePtr<asset::AssetManager> m_assetManager{nullptr};

		friend class ProjectSerializer;
	};

	class TST_API ProjectSerializer
	{
	public:
		ProjectSerializer(Project *p_project);

		// Serializes the project to the path provided in the project's constructor (Project::m_path)
		auto serialize() const -> void;

		// Most of the time you will be using deserialize because it is uncommon to create a new project at runtime unless you are writing a launcher
		auto deserialize(const io::filesystem::Path &p_path) -> void;

	private:
		NonOwningPtr<Project> m_project{nullptr};
	};
}
