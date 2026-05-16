#include "runtime_application.hpp"

#include <argparse/argparse.hpp>

#include "toast_lib/logging.hpp"
#include "toast_lib/os/file_dialog.hpp"
#include "toast_lib/os/terminal.hpp"

#if USE_WINMAIN
INT WINAPI WinMain([[maybe_unused]] HINSTANCE hInstance, [[maybe_unused]] HINSTANCE hPrevInstance, [[maybe_unused]] LPSTR lpCmdLine, [[maybe_unused]] INT nCmdShow)
{
#else
auto main(int32 p_argc, char **p_argv) -> int32
{
	#endif
	auto binary_directory{toaster::os::getBinaryDirectory()};

	argparse::ArgumentParser parser{"Toaster", "3.141592653589793284"};
	parser.add_argument("--scriptAsm").help("The path to the C# script assembly DLL").default_value(toaster::io::filesystem::Path{
																										binary_directory /
																										"../examples/Sandbox/bin/Debug/net48/Sandbox.dll"
																									}.string());
	parser.add_argument("--scene").help("The startup scene (.tscene)").default_value("__NONE__");
	parser.parse_args(__argc, __argv);

	toaster::ApplicationCreateInfo app_create_info{};
	app_create_info.windowCreateInfo.width          = 1920;
	app_create_info.windowCreateInfo.height         = 1080;
	app_create_info.windowCreateInfo.title          = "Toaster Vπ - Runtime";
	app_create_info.windowCreateInfo.iconPath       = binary_directory / "../resources/textures/OrboCloseup.png";
	app_create_info.windowCreateInfo.startMaximized = true;

	{
		toaster::RuntimeApplication app{app_create_info, &parser};
		app.run();
	}

	return 0;
}
