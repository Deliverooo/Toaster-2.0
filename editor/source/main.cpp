#include "editor_application.hpp"
#include "argparse/argparse.hpp"
#include "toast_lib/logging.hpp"
#include "toast_lib/os/terminal.hpp"

#if USE_WINMAIN
INT WINAPI WinMain([[maybe_unused]] HINSTANCE hInstance, [[maybe_unused]] HINSTANCE hPrevInstance, [[maybe_unused]] LPSTR lpCmdLine, [[maybe_unused]] INT nCmdShow)
{
#else
int main(int32 p_argc, char **p_argv) // Maybe_todo, Forward these parameters to the application for it to handle
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

	std::unordered_map<toaster::String, toaster::String> command_line_args;
	command_line_args["scriptAsm"]    = parser.get<toaster::String>("--scriptAsm");
	command_line_args["startupScene"] = parser.get<toaster::String>("--scene");

	toaster::ApplicationCreateInfo app_create_info{};
	app_create_info.windowCreateInfo.width          = 1920;
	app_create_info.windowCreateInfo.height         = 1080;
	app_create_info.windowCreateInfo.title          = "Toaster Vπ - Runtime";
	app_create_info.windowCreateInfo.iconPath       = binary_directory / "../resources/textures/OrboCloseup.png";
	app_create_info.windowCreateInfo.startMaximized = true;

	toaster::EditorApplication app{app_create_info, command_line_args};
	app.run();

	return 0;
}
