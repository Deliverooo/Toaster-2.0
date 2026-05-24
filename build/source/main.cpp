#include <argparse/argparse.hpp>

#include "toast_lib/logging.hpp"
#include "toast_lib/toast_assert.h"
#include "toast_lib/os/terminal.hpp"
#include "toast_project/project.hpp"

using namespace toaster;

constexpr auto c_dotnetFrameworkVersion{"net48"};
constexpr auto c_dotnetLanguageVersion{"10.0"};
constexpr auto c_dotnetProfile{"Debug"};

constexpr auto c_csprojTemplate{
	R"(<Project Sdk="Microsoft.NET.Sdk">
	<PropertyGroup>
		<TargetFramework>{0}</TargetFramework>
		<LangVersion>{1}</LangVersion>
		<ImplicitUsings>enable</ImplicitUsings>
		<Nullable>enable</Nullable>
	</PropertyGroup>
<ItemGroup>
  <Reference Include="Toaster">
    <HintPath>{2}</HintPath>
  </Reference>
</ItemGroup>
</Project>)"
};

constexpr auto c_tsceneTemplate{
	R"(Scene:
  Name: {0}
  SceneEnvironmentAssetID: 5772156649
  Entities:
    - Entity: 2718281828459
      TagComponent:
        Tag: Camera Controller
      TransformComponent:
        Translation: [0, 0, 0]
        Rotation: [1, 0, 0, 0]
        Scale: [1, 1, 1]
      ScriptComponent:
        ClassName: Toaster.CameraController)"
};

constexpr auto c_tprojTemplate{
	R"(Project:
  Name: {0}
  AssetRegistryPath: resources/asset_registry.treg
  ScriptDirectory: resources/scripts
  SceneDirectory: resources/scenes
  MeshDirectory: resources/meshes
  TextureDirectory: resources/textures
  StartupSceneName: {1})"
};

constexpr auto c_assetRegistryTemplate{
	R"(AssetRegistry:
  [])"
};

constexpr auto c_gitignoreData{
	R"(.vs/
.idea/
.vscode/
resources/scripts/bin/
resources/scripts/obj/
)"
};

auto newProject(const argparse::ArgumentParser &p_new_command) -> int32
{
	io::filesystem::Path toaster_dll{fmt::format("{0}/script/{1}/{2}/Toaster.dll", os::getBinaryDirectory(), c_dotnetProfile, c_dotnetFrameworkVersion)};
	if (!std::filesystem::exists(toaster_dll))
	{
		LOG_ERROR("Toaster.dll does not exist at '{}'. Please run build_scripts.bat", toaster_dll);
		return -1;
	}

	auto project_name{p_new_command.get<String>("--name")};
	auto scene_name{p_new_command.get<String>("--sceneName")};

	LOG_INFO("Creating new project: '{}'", project_name);

	#pragma region create directories

	#define CREATE_DIRECTORY(__path, __info)\
			LOG_INFO("Creating {} directory", #__info); do { if (!std::filesystem::create_directory(__path)) {\
			LOG_ERROR("Directory already exists or creation failed"); return -1; } } while(false)

	const io::filesystem::Path project_root{project_name};
	CREATE_DIRECTORY(project_root, project root);
	const io::filesystem::Path resource_directory{project_root / "resources"};

	CREATE_DIRECTORY(resource_directory, resources);
	CREATE_DIRECTORY(resource_directory / "scripts", scripts);
	CREATE_DIRECTORY(resource_directory / "scenes", scenes);
	CREATE_DIRECTORY(resource_directory / "meshes", meshes);
	CREATE_DIRECTORY(resource_directory / "textures", textures);
	CREATE_DIRECTORY(resource_directory / "environments", environments);

	#undef CREATE_DIRECTORY
	#pragma endregion

	// Serializing the project with the project serializer class would require a valid project, which would require a valid render::RenderContext.
	LOG_INFO("Creating .tproj");
	String project_data{fmt::format(c_tprojTemplate, project_name, scene_name)};
	io::filesystem::writeFile(fmt::format("{0}.tproj", project_root / project_name), project_data);

	LOG_INFO("Creating asset registry");
	io::filesystem::writeFile(fmt::format("{0}/resources/asset_registry.treg", project_root), c_assetRegistryTemplate);

	LOG_INFO("Creating .csproj");
	// Create the .csproj manually so I can use net48, then build with dotnet
	io::filesystem::writeFile(resource_directory / "scripts" / fmt::format("{0}.csproj", project_name),
							  fmt::format(c_csprojTemplate, c_dotnetFrameworkVersion, c_dotnetLanguageVersion, toaster_dll));

	LOG_INFO("Building C# assembly");
	String build_scripts_command{fmt::format("cd {0} && dotnet build", io::filesystem::Path{resource_directory / "scripts"})};
	int32  err{std::system(build_scripts_command.c_str())};
	if (err == -1)
	{
		LOG_ERROR("Failed to build script projects!");
		return -1;
	}

	LOG_INFO("Creating default scene");
	io::filesystem::writeFile(resource_directory / "scenes" / fmt::format("{}.tscene", scene_name), fmt::format(c_tsceneTemplate, scene_name));

	LOG_INFO("Creating utility build scripts");
	io::filesystem::writeFile(project_root / "build_assets.bat", "tstb buildAssets");

	LOG_INFO("Creating .gitignore");
	io::filesystem::writeFile(project_root / ".gitignore", c_gitignoreData);

	return 0;
}

auto removeProject(const argparse::ArgumentParser &p_remove_command) -> int32
{
	io::filesystem::Path project_dir{p_remove_command.get("--name")};
	if (!io::filesystem::exists(project_dir))
	{
		LOG_ERROR("Project directory '{}' does not exist", project_dir);
	}

	String confirm{"N"};
	LOG_WARN("Are you sure?: (Y / N)");
	std::cin >> confirm;
	if (confirm == "N" || confirm == "n")
	{
		LOG_INFO("ok...");
		return 0;
	}
	if (confirm == "Y" || confirm == "y")
	{
		std::error_code err{};
		std::uintmax_t  remove_count{std::filesystem::remove_all(project_dir, err)};
		if (err)
		{
			LOG_ERROR("Failed to remove project | Error: {}", err.message());
			return -1;
		}

		LOG_INFO("Successfully removed project: {} files", remove_count);
		return 0;
	}
	LOG_ERROR("Invalid option.");
	return -1;
}

auto getTProjPath() -> io::filesystem::Path
{
	const auto working_directory{std::filesystem::current_path()};

	io::filesystem::Path tproj_file_path{};
	for (const auto &dir_it: std::filesystem::directory_iterator{working_directory})
	{
		if (dir_it.is_regular_file())
		{
			if (dir_it.path().extension() == ".tproj")
			{
				LOG_INFO("Found tproj file: {}", dir_it.path());
				tproj_file_path = dir_it.path();
			}
		}
	}
	return tproj_file_path;
}

auto updateAssetRegistry(const io::filesystem::Path &p_tproj_path) -> int32
{
	Project           project{};
	ProjectSerializer project_serializer{&project};
	project_serializer.deserialize(p_tproj_path);
	auto &asset_manager{project.getAssetManager()};

	asset_manager.deserializeFromFile(project.getFullAssetRegistryPath());
	std::unordered_set<io::filesystem::Path> asset_paths_set;
	for (const auto &dir_it: std::filesystem::recursive_directory_iterator{project.getRootDirectory()})
	{
		if (dir_it.is_regular_file())
		{
			io::filesystem::EFileType file_type{io::filesystem::getFileType(dir_it.path())};
			if (file_type == io::filesystem::EFileType::eImage || file_type == io::filesystem::EFileType::eMesh || file_type ==
				io::filesystem::EFileType::eEnvironmentMap)
			{
				asset_paths_set.insert(std::filesystem::relative(dir_it.path(), project.getRootDirectory()));
			}
		}
	}

	for (const auto &path: asset_paths_set)
	{
		LOG_INFO("{}", path);
	}

	for (auto &[id, metadata]: asset_manager.getAssetMetadataRegistry())
	{
		// Asset path is invalid, so remove
		if (!asset_paths_set.contains(metadata.path))
			asset_manager.removeAsset(id);
	}

	asset_manager.printAssetRegistry();

	asset_manager.serializeToFile(project.getFullAssetRegistryPath());

	return 0;
}

auto buildAssets(const argparse::ArgumentParser &p_build_assets_command) -> int32
{
	LOG_INFO("Attempting to build assets");
	io::filesystem::Path tproj_file_path{getTProjPath()};
	if (!std::filesystem::exists(tproj_file_path))
	{
		LOG_ERROR("Current directory does not contain a .tproj file!");
		return -1;
	}

	Project           project{};
	ProjectSerializer project_serializer{&project};
	project_serializer.deserialize(tproj_file_path);
	auto &asset_manager{project.getAssetManager()};

	asset_manager.deserializeFromFile(project.getFullAssetRegistryPath());

	// Remove any assets whose path don't exist if flag is specified
	if (p_build_assets_command.get<bool>("--removeInvalid"))
		asset_manager.removeAssetsWithInvalidPaths();

	// We don't want to add assets that are already in the registry, so skip them.
	for (const auto &dir_it: std::filesystem::recursive_directory_iterator{project.getRootDirectory()})
	{
		if (dir_it.is_regular_file())
		{
			// The asset registry stores paths relative to the root directory of the project
			io::filesystem::Path registry_asset_path{std::filesystem::relative(dir_it.path(), project.getRootDirectory())};
			if (asset_manager.hasAnyAssetsWithPath(registry_asset_path))
				continue;

			switch (io::filesystem::getFileType(dir_it.path()))
			{
				case io::filesystem::EFileType::eImage:
				{
					asset::AssetMetadata metadata{registry_asset_path, asset::EAssetType::eTexture2D};
					asset_manager.addAssetMetadata({}, metadata);
					LOG_INFO("Found image asset file: {}", metadata.path);
					break;
				}
				case io::filesystem::EFileType::eEnvironmentMap:
				{
					asset::AssetMetadata metadata{registry_asset_path, asset::EAssetType::eTexture3D};
					asset_manager.addAssetMetadata({}, metadata);
					LOG_INFO("Found environment map asset file: {}", metadata.path);
					break;
				}
				case io::filesystem::EFileType::eMesh:
				{
					asset::AssetMetadata metadata{registry_asset_path, asset::EAssetType::eMesh};
					asset_manager.addAssetMetadata({}, metadata);
					LOG_INFO("Found mesh asset file: {}", metadata.path);
					break;
				}
				default: break;
			}
		}
	}

	// Serialize the new registry to the original path
	asset_manager.serializeToFile(project.getFullAssetRegistryPath());

	// This is the new asset registry
	asset_manager.printAssetRegistry();

	return 0;
}

auto buildAssemblies(const argparse::ArgumentParser &p_build_command) -> int32
{
	LOG_INFO("Attempting to build project");

	const auto working_directory{std::filesystem::current_path()};

	io::filesystem::Path tproj_file_path{getTProjPath()};
	if (!std::filesystem::exists(tproj_file_path))
	{
		LOG_ERROR("Current directory does not contain a .tproj file!");
		return -1;
	}

	Project           project{};
	ProjectSerializer project_serializer{&project};
	project_serializer.deserialize(tproj_file_path);

	LOG_INFO("Building C# assembly");
	String build_scripts_command{fmt::format("cd {0} && dotnet build", project.getFullScriptDirectory())};
	int32  err{std::system(build_scripts_command.c_str())};
	if (err == -1)
	{
		LOG_ERROR("Failed to build script projects!");
		return -1;
	}

	return buildAssets(p_build_command);
}

auto main(int32 p_argc, char **p_argv) -> int32
{
	const auto working_directory{os::getBinaryDirectory()};

	argparse::ArgumentParser parser{"Toaster build (tstb)", "2.718281828"};

	argparse::ArgumentParser new_command{"new"};
	new_command.add_description("Create a new project");
	new_command.add_argument("--name", "-n").help("The name of the project to create").default_value("New_Project");
	new_command.add_argument("--sceneName", "-sn").help("The name of the default scene to create").default_value("New_Scene");
	parser.add_subparser(new_command);

	argparse::ArgumentParser remove_command{"rm"};
	remove_command.add_description("remove a project");
	remove_command.add_argument("--name", "-n").help("The name of the project to remove").required();
	parser.add_subparser(remove_command);

	argparse::ArgumentParser build_assets_command{"buildAssets"};
	build_assets_command.add_description("build the meta asset files for a project");
	build_assets_command.add_argument("--removeInvalid").default_value(false).implicit_value(true).help("Remove any assets whose path don't exist if flag is specified");
	parser.add_subparser(build_assets_command);

	argparse::ArgumentParser build_command{"buildAssemblies"};
	build_command.add_description("build the C# assemblies aswell as the assets for a project");
	parser.add_subparser(build_command);

	argparse::ArgumentParser update_command{"update"};
	update_command.add_description("update the asset registry and remove unused assets for a project");
	parser.add_subparser(update_command);

	try
	{
		parser.parse_args(p_argc, p_argv);
	}
	catch (const std::exception &err)
	{
		LOG_ERROR("Bradar, wat is dis?: {}", err.what());
		return -1;
	}

	if (parser.is_subcommand_used("new"))
		return newProject(new_command);

	if (parser.is_subcommand_used("rm"))
		return removeProject(remove_command);

	if (parser.is_subcommand_used("buildAssets"))
		return buildAssets(build_assets_command);

	if (parser.is_subcommand_used("buildAssemblies"))
		return buildAssemblies(build_command);

	if (parser.is_subcommand_used("update"))
	{
		io::filesystem::Path tproj_file_path{getTProjPath()};
		if (!std::filesystem::exists(tproj_file_path))
		{
			LOG_ERROR("Current directory does not contain a .tproj file!");
			return -1;
		}
		return updateAssetRegistry(tproj_file_path);
	}

	LOG_WARN("What are you trying to do? use the new command or type --help for help");
	return 0;
}
