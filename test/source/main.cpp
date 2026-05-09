#include <iostream>

#include "toaster/toast_scripting/script_engine.hpp"

auto main(int32 p_argc, char **p_argv) -> int32
{
	std::vector<toaster::String> command_line_args;
	command_line_args.resize(p_argc);
	for (uint32 i{0u}; i < p_argc; ++i)
		command_line_args[i] = p_argv[i];

	toaster::io::filesystem::Path binary_dir{command_line_args[0]};
	binary_dir = binary_dir.parent_path();

	toaster::io::filesystem::Path core_script_assembly_dll{binary_dir / "../test/script/bin/Debug/net10.0/Test.dll"};

	toaster::script::clr::CLRScriptEngineSpecInfo clr_script_engine_spec_info{};
	clr_script_engine_spec_info.coreAssemblyPath = core_script_assembly_dll;
	toaster::script::clr::CLRScriptEngine clr_script_engine{clr_script_engine_spec_info};

	std::cin.get();

	return 0;
}
