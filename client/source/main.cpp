#include "client_application.hpp"

int main(int argc, char **argv)
{
	toaster::ClientApplication *app = new toaster::ClientApplication();
	app->run();
	delete app;

	return 0;
}
