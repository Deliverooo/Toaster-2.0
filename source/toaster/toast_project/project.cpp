#include "project.hpp"

#include "toast_lib/toast_assert.h"

#include <yaml-cpp/yaml.h>

#include "toast_lib/logging.hpp"

namespace toaster
{
	Project::Project()
	{
		m_assetManager = make_unique<asset::AssetManager>(this);
	}

	Project::Project(render::RenderContext *p_render_ctx)
	{
		m_assetManager = make_unique<asset::AssetManager>(this, p_render_ctx);
	}

	auto Project::getFullAssetRegistryPath() const -> io::filesystem::Path
	{
		return getRootDirectory() / m_specInfo.assetRegistryPath;
	}

	auto Project::getFullScriptDirectory() const -> io::filesystem::Path
	{
		return getRootDirectory() / m_specInfo.scriptDirectory;
	}

	auto Project::getFullSceneDirectory() const -> io::filesystem::Path
	{
		return getRootDirectory() / m_specInfo.sceneDirectory;
	}

	auto Project::getFullMeshDirectory() const -> io::filesystem::Path
	{
		return getRootDirectory() / m_specInfo.meshDirectory;
	}

	auto Project::getFullTextureDirectory() const -> io::filesystem::Path
	{
		return getRootDirectory() / m_specInfo.textureDirectory;
	}

	auto Project::getPath() const -> const io::filesystem::Path &
	{
		return m_path;
	}

	auto Project::getRootDirectory() const -> io::filesystem::Path
	{
		return m_path.parent_path();
	}

	auto Project::getSpecInfo() const -> const ProjectSpecInfo &
	{
		return m_specInfo;
	}

	auto Project::getAssetManager() const -> asset::AssetManager &
	{
		return *m_assetManager;
	}

	auto Project::printInfo() const -> void
	{
		LOG_INFO("Project: '{}' [", m_path);
		LOG_TRACE("\tName: {}", m_specInfo.name);
		LOG_TRACE("\tScript directory: {}", m_specInfo.scriptDirectory);
		LOG_TRACE("\tScene directory: {}", m_specInfo.sceneDirectory);
		LOG_TRACE("\tMesh directory: {}", m_specInfo.meshDirectory);
		LOG_TRACE("\tStartup scene name: {}", m_specInfo.startupSceneName);
		LOG_INFO("]");
	}

	ProjectSerializer::ProjectSerializer(Project *p_project) : m_project(p_project)
	{
		TST_PERMA_ASSERT_MSG(m_project, "Project cannot be null because it will be read/written to");
	}

	auto ProjectSerializer::serialize() const -> void
	{
		const ProjectSpecInfo &project_spec_info{m_project->m_specInfo};
		const auto &           path{m_project->m_path};

		YAML::Emitter out{};

		out << YAML::BeginMap;
		out << YAML::Key << "Project" << YAML::BeginMap;

		out << YAML::Key << "Name" << YAML::Value << project_spec_info.name;
		out << YAML::Key << "AssetRegistryPath" << YAML::Value << project_spec_info.assetRegistryPath;
		out << YAML::Key << "ScriptDirectory" << YAML::Value << project_spec_info.scriptDirectory;
		out << YAML::Key << "SceneDirectory" << YAML::Value << project_spec_info.sceneDirectory;
		out << YAML::Key << "MeshDirectory" << YAML::Value << project_spec_info.meshDirectory;
		out << YAML::Key << "TextureDirectory" << YAML::Value << project_spec_info.textureDirectory;
		out << YAML::Key << "StartupSceneName" << YAML::Value << project_spec_info.startupSceneName;

		out << YAML::EndMap;
		out << YAML::EndMap;

		io::filesystem::writeFile(path, out.c_str());
	}

	auto ProjectSerializer::deserialize(const io::filesystem::Path &p_path) -> void
	{
		m_project->m_path = p_path;
		ProjectSpecInfo &project_spec_info{m_project->m_specInfo};

		const String yaml_string{io::filesystem::readFile(p_path)};
		YAML::Node   data{YAML::Load(yaml_string)};

		auto project_node{data["Project"]};
		if (!project_node)
		{
			LOG_ERROR("Failed to deserialize project: {}", p_path);
			return;
		}

		project_spec_info.name              = project_node["Name"].as<String>();
		project_spec_info.assetRegistryPath = project_node["AssetRegistryPath"].as<String>();
		project_spec_info.scriptDirectory   = project_node["ScriptDirectory"].as<String>();
		project_spec_info.sceneDirectory    = project_node["SceneDirectory"].as<String>();
		project_spec_info.meshDirectory     = project_node["MeshDirectory"].as<String>();
		project_spec_info.meshDirectory     = project_node["TextureDirectory"].as<String>();
		project_spec_info.startupSceneName  = project_node["StartupSceneName"].as<String>();
	}
}
