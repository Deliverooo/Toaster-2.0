#include <argparse/argparse.hpp>

#include "toast_lib/logging.hpp"
#include "toast_lib/toast_assert.h"
#include "toast_lib/os/terminal.hpp"

#include "tstb.hpp"

using namespace toaster;

auto main(int32 p_argc, char **p_argv) -> int32
{
	const auto working_directory{os::getBinaryDirectory()};

	argparse::ArgumentParser parser{"Toaster build (tstb)", "2.718281828"};

	argparse::ArgumentParser new_command{"new"};
	new_command.add_description("Create a new thing");

	argparse::ArgumentParser new_cpp_project_cmd{"cppProj"};
	new_cpp_project_cmd.add_description("Creates a new C++ 'project' with a basic framework to build a game");
	new_cpp_project_cmd.add_argument("--name", "-n").help("The name of the project/executable").default_value("New_Cpp_Project");
	new_command.add_subparser(new_cpp_project_cmd);

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

	// The tstb
	tstb::TstB tstb{};

	if (parser.is_subcommand_used("new"))
	{
		if (new_command.is_subcommand_used("cppProj"))
			return tstb.newCppProj(new_cpp_project_cmd);
	}

	LOG_WARN("What are you trying to do? use the new command or type --help for help");
	return 0;
}
