#include "client_application.hpp"

#if USE_WINMAIN
INT WINAPI WinMain([[maybe_unused]] HINSTANCE hInstance, [[maybe_unused]] HINSTANCE hPrevInstance, [[maybe_unused]] LPSTR lpCmdLine, [[maybe_unused]] INT nCmdShow)
{
#else
int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) // Maybe_todo, Forward these parameters to the application for it to handle
{
	#endif

	toaster::ApplicationCreateInfo app_create_info{};
	app_create_info.windowCreateInfo.width          = 1280u;
	app_create_info.windowCreateInfo.height         = 720u;
	app_create_info.windowCreateInfo.title          = "Toaster v3.1415 - Vulkan";
	app_create_info.windowCreateInfo.iconPath       = "../resources/textures/OrboCloseup.png";
	app_create_info.windowCreateInfo.startMaximized = false;

	auto *app = new toaster::ClientApplication(app_create_info);

	app->run();
	delete app;
	return EXIT_SUCCESS;
}
