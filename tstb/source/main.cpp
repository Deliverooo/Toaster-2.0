#include <argparse/argparse.hpp>

#include "toast_lib/logging.hpp"
#include "toast_lib/toast_assert.h"
#include "toast_lib/os/terminal.hpp"
#include "toast_project/project.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "tstb.hpp"

using namespace toaster;

auto main(int32 p_argc, char **p_argv) -> int32
{
	const auto working_directory{os::getBinaryDirectory()};

	argparse::ArgumentParser parser{"Toaster build (tstb)", "2.718281828"};

	argparse::ArgumentParser new_command{"new"};
	new_command.add_description("Create a new thing");

	argparse::ArgumentParser new_project_cmd{"project"};
	new_project_cmd.add_description("Create a new project");
	new_project_cmd.add_argument("--name", "-n").help("The name of the project to create").default_value("New_Project");
	new_project_cmd.add_argument("--sceneName", "-sn").help("The name of the default scene to create").default_value("New_Scene");
	new_command.add_subparser(new_project_cmd);

	argparse::ArgumentParser new_mesh_cmd{"mesh"};
	new_mesh_cmd.add_description("Create a new .tmesh file");
	new_mesh_cmd.add_argument("--srcPath", "-sp").help("The path to the mesh source file").required();
	new_command.add_subparser(new_mesh_cmd);

	argparse::ArgumentParser new_cpp_project_cmd{"cppProj"};
	new_cpp_project_cmd.add_description("Creates a new C++ 'project' with a basic framework to build a game");
	new_cpp_project_cmd.add_argument("--name", "-n").help("The name of the project/executable").default_value("New_Cpp_Project");
	new_command.add_subparser(new_cpp_project_cmd);

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

	try
	{
		parser.parse_args(p_argc, p_argv);
	}
	catch (const std::exception &err)
	{
		LOG_ERROR("Bradar, wat is dis?: {}", err.what());
		return -1;
	}

	// The tstb
	tstb::TstB tstb{tstb::TstB::tryGetTProjPath()};

	if (parser.is_subcommand_used("new"))
	{
		if (new_command.is_subcommand_used("project"))
			return tstb.newProject(new_project_cmd);

		if (new_command.is_subcommand_used("cppProj"))
			return tstb.newCppProj(new_cpp_project_cmd);

		if (new_command.is_subcommand_used("mesh"))
			return tstb.newMesh(new_mesh_cmd);
	}

	if (parser.is_subcommand_used("rm"))
		return tstb.removeProject(remove_command);

	if (parser.is_subcommand_used("buildAssets"))
		return tstb.buildAssets(build_assets_command);

	if (parser.is_subcommand_used("buildAssemblies"))
		return tstb.buildAssemblies(build_command);

	LOG_WARN("What are you trying to do? use the new command or type --help for help");
	return 0;
}
