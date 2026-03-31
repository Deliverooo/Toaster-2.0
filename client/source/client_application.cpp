#include "client_application.hpp"
#include "client_layer.hpp"

namespace toaster
{
	ClientApplication::ClientApplication(const ApplicationCreateInfo& p_create_info) : Application(p_create_info)
	{
		io::filesystem::setWorkingDirectory("../"); // The main Toaster dir (where the resource folder is)


		addLayer(new ClientLayer(this));
	}

	ClientApplication::~ClientApplication()
	{
	}
}
