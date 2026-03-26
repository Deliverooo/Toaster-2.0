#include "editor_application.hpp"

#if USE_WINMAIN
INT WINAPI WinMain([[maybe_unused]] HINSTANCE hInstance, [[maybe_unused]] HINSTANCE hPrevInstance, [[maybe_unused]] LPSTR lpCmdLine, [[maybe_unused]] INT nCmdShow)
{
#else
int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) // Maybe_todo, Forward these parameters to the application for it to handle
{
	#endif

	auto *app = new toaster::EditorApplication();

	app->run();
	delete app;
	return EXIT_SUCCESS;
}
