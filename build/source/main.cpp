#include <argparse/argparse.hpp>

#include "toast_lib/logging.hpp"
#include "toast_lib/toast_assert.h"
#include "toast_lib/os/terminal.hpp"

using namespace toaster;

constexpr auto c_csprojTemplate{
	R"(<Project Sdk="Microsoft.NET.Sdk">
	<PropertyGroup>
		<TargetFramework>net48</TargetFramework>
		<LangVersion>10.0</LangVersion>
		<ImplicitUsings>enable</ImplicitUsings>
		<Nullable>enable</Nullable>
	</PropertyGroup>
<ItemGroup>
  <Reference Include="Toaster">
    <HintPath>{0}</HintPath>
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
	// LOG_INFO("Working dir: {}", working_directory.string());

	argparse::ArgumentParser parser{"Toaster launcher", "2.718281828"};

	argparse::ArgumentParser new_command{"new"};
	new_command.add_description("Create a new project");
	new_command.add_argument("--name", "-n").help("The name of the project to create").required();
	new_command.add_argument("--sceneName", "-sn").help("The name of the default scene to create").default_value("New_Scene");

	parser.add_subparser(new_command);

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
		if (!std::filesystem::exists(working_directory / "script/Toaster.dll"))
		{
			LOG_ERROR("No toaster assembly was found!");
			return -1;
		}
		auto project_name{new_command.get<String>("-n")};

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

		io::filesystem::writeFile(project_root / fmt::format("{}.tproj", project_name), "Orbo is sigma!");

		LOG_INFO("Creating .csproj");
		// Create the .csproj manually so I can use net48, then build with dotnet
		io::filesystem::writeFile(resource_directory / "scripts" / fmt::format("{}.csproj", project_name),
								  fmt::format(c_csprojTemplate, io::filesystem::Path{working_directory / "script/Toaster.dll"}.string()));
		LOG_INFO("Building C# assembly");
		String build_scripts_command{fmt::format("cd {} && dotnet build", io::filesystem::Path{resource_directory / "scripts"}.string())};
		int32  err{std::system(build_scripts_command.c_str())};
		if (err == -1)
		{
			LOG_ERROR("Failed to build script projects!");
			return -1;
		}

		auto scene_name{new_command.get<String>("--sceneName")};
		LOG_INFO("Creating default scene");
		io::filesystem::writeFile(resource_directory / "scenes" / fmt::format("{}.tscene", scene_name), fmt::format(c_tsceneTemplate, scene_name));
	}
	else
	{
		LOG_WARN("What are you trying to do? use the new command or type --help for help");
		return 0;
	}

	return 0;
}
