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
	R"(Scene: {0}
Entities:
  - Entity: 67
    TagComponent:
      Tag: Camera Controller
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [1, 0, 0, 0]
      Scale: [1, 1, 1]
    ScriptComponent:
      ClassName: Toaster.CameraController)"
};

auto main(int32 p_argc, char **p_argv) -> int32
{
	const auto working_directory{os::getBinaryDirectory()};

	argparse::ArgumentParser parser{"Toaster build (tstb)", "2.718281828"};

	argparse::ArgumentParser new_command{"new"};
	new_command.add_description("Create a new project");
	new_command.add_argument("--name", "-n").help("The name of the project to create").default_value("New_Project");
	new_command.add_argument("--sceneName", "-sn").help("The name of the default scene to create").default_value("New_Scene");

	argparse::ArgumentParser remove_command{"rm"};
	remove_command.add_description("remove a project");
	remove_command.add_argument("--name", "-n").help("The name of the project to remove").required();

	parser.add_subparser(new_command);
	parser.add_subparser(remove_command);

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
	{
		io::filesystem::Path toaster_dll{fmt::format("{0}/script/{1}/{2}/Toaster.dll", working_directory, c_dotnetProfile, c_dotnetFrameworkVersion)};
		if (!std::filesystem::exists(toaster_dll))
		{
			LOG_ERROR("Toaster.dll does not exist at '{}'. Please run build_scripts.bat", toaster_dll);
			return -1;
		}

		auto project_name{new_command.get<String>("--name")};
		auto scene_name{new_command.get<String>("--sceneName")};

		#pragma region create directories
		LOG_INFO("Creating new project: '{}'", project_name);
		io::filesystem::Path project_root{project_name};
		if (!std::filesystem::create_directory(project_root))
		{
			LOG_ERROR("Directory already exists or creation failed");
			return -1;
		}
		io::filesystem::Path resource_directory{project_root / "resources"};

		LOG_INFO("Creating resources directory");
		if (!std::filesystem::create_directory(resource_directory))
		{
			LOG_ERROR("Directory already exists or creation failed");
			return -1;
		}

		LOG_INFO("Creating scripts directory");
		if (!std::filesystem::create_directory(resource_directory / "scripts"))
		{
			LOG_ERROR("Directory already exists or creation failed");
			return -1;
		}

		LOG_INFO("Creating scenes directory");
		if (!std::filesystem::create_directory(resource_directory / "scenes"))
		{
			LOG_ERROR("Directory already exists or creation failed");
			return -1;
		}

		LOG_INFO("Creating meshes directory");
		if (!std::filesystem::create_directory(resource_directory / "meshes"))
		{
			LOG_ERROR("Directory already exists or creation failed");
			return -1;
		}
		#pragma endregion

		// Serialize the new project
		ProjectSpecInfo project_spec_info{};
		project_spec_info.name             = project_name;
		project_spec_info.startupSceneName = scene_name;
		Project           new_project{fmt::format("{0}.tproj", project_root / project_name), project_spec_info};
		ProjectSerializer project_serializer{&new_project};
		project_serializer.serialize();

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
	}
	else if (parser.is_subcommand_used("rm"))
	{
		io::filesystem::Path project_dir{remove_command.get("--name")};
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
		else if (confirm == "Y" || confirm == "y")
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
		else
		{
			LOG_ERROR("Invalid option.");
			return -1;
		}
	}
	else
	{
		LOG_WARN("What are you trying to do? use the new command or type --help for help");
		return 0;
	}

	return 0;
}
