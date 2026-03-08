#include "editor_application.hpp"

#if USE_WINMAIN
INT WINAPI WinMain([[maybe_unused]] HINSTANCE hInstance, [[maybe_unused]] HINSTANCE hPrevInstance, [[maybe_unused]] LPSTR lpCmdLine, [[maybe_unused]] INT nCmdShow)
{
#else
int main(int argc, char **argv)
{
	#endif

	auto *app = new toaster::EditorApplication();

	app->run();
	delete app;
	return EXIT_SUCCESS;
}
