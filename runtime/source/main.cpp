#include "toast_kernel/application.hpp"

#include <argparse/argparse.hpp>

#include "runtime_layer.hpp"
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

	// toaster::log::setOutputFile((binary_directory / "runtime_log.tlog").string());

	argparse::ArgumentParser parser{"Toaster", "3.141592653589793284"};

	parser.add_argument("--project", "-p").help("The path to the Toaster project file (.tproj)").
			default_value("C:/dev/Toaster-2.0/examples/New_Project/New_Project.tproj");

	parser.parse_args(p_argc, p_argv);

	toaster::ApplicationSpecInfo app_create_info{};
	app_create_info.windowSpecInfo.size           = {1920u, 1080u};
	app_create_info.windowSpecInfo.title          = "Toaster Vπ - Runtime";
	app_create_info.windowSpecInfo.iconPath       = binary_directory / "../resources/textures/OrboCloseup.png";
	app_create_info.windowSpecInfo.startMaximized = true;
	#ifndef NDEBUG
	app_create_info.printGPUDebugInfo = true;
	#else
	app_create_info.printGPUDebugInfo = false;
	#endif

	{
		toaster::Application app{app_create_info, &parser};
		app.addLayer<toaster::RuntimeLayer>();
		app.run();
	}

	// toaster::log::shutdown();

	return 0;
}
