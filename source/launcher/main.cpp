#include "application.hpp"
#include "logging.hpp"

#if USE_WINMAIN
INT WINAPI WinMain([[maybe_unused]] HINSTANCE hInstance, [[maybe_unused]] HINSTANCE hPrevInstance, [[maybe_unused]] LPSTR lpCmdLine, [[maybe_unused]] INT nCmdShow)
{
#else
int main(int argc, char **argv)
{
	#endif

	toaster::Application application{};

	try
	{
		application.run();
	}
	catch (const std::exception &e)
	{
		LOG_ERROR("{}", e.what());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
