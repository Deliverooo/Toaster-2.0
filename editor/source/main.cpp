#include "editor_application.hpp"

#if USE_WINMAIN
INT WINAPI WinMain([[maybe_unused]] HINSTANCE hInstance, [[maybe_unused]] HINSTANCE hPrevInstance, [[maybe_unused]] LPSTR lpCmdLine, [[maybe_unused]] INT nCmdShow)
{
#else
int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) // Maybe_todo, Forward these parameters to the application for it to handle
{
	#endif

	toaster::ApplicationCreateInfo app_create_info{};
	app_create_info.windowCreateInfo.width          = 1920;
	app_create_info.windowCreateInfo.height         = 1080;
	app_create_info.windowCreateInfo.title          = "Toaster v3.1415 - vulkan";
	app_create_info.windowCreateInfo.iconPath       = "../resources/textures/OrboCloseup.png";
	app_create_info.windowCreateInfo.startMaximized = true;

	auto *app = new toaster::EditorApplication(app_create_info);

	app->run();
	delete app;
	return EXIT_SUCCESS;
}
