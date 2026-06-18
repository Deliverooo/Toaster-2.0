#include "img/new_layer.hpp"

#ifndef NDEBUG
auto main(int32 p_argc, char **p_argv) -> int32
{
	#else
	#ifndef _WINDOWS_
	#include <Windows.h>
	#undef min // Why windows? :(
	#undef max
	#endif
	INT APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
	{


	#endif

	char *toaster_sdk_dir{std::getenv("TOASTER_SDK")};

	if (!toaster_sdk_dir)
		TST_PERMA_ASSERT_MSG(false, "Dondé está Toaster? If you just installed the SDK, please restart your computer to apply the environment variable settings.");

	argparse::ArgumentParser command_line_args{"img"};
	command_line_args.add_argument("--image", "-i").help("The path of the image to view").default_value("");

	command_line_args.parse_args(__argc, __argv);

	tst::ApplicationSpecInfo app_spec{};
	app_spec.printGPUDebugInfo             = false;
	app_spec.windowSpecInfo.title          = "img";
	app_spec.windowSpecInfo.startMaximized = true;
	app_spec.sdkDir                        = toaster_sdk_dir;
	tst::Application app{app_spec, &command_line_args};

	app.addLayer<img::NewLayer>();

	// Run the app!!!
	app.run();

	return 0;
}
