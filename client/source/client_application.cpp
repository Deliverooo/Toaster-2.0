#include "client_application.hpp"
#include "client_layer.hpp"

namespace toaster
{
	ClientApplication::ClientApplication() : Application()
	{
		addLayer(new ClientLayer(this));
	}

	ClientApplication::~ClientApplication()
	{
	}
}
