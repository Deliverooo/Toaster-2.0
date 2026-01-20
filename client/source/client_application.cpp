#include "client_application.hpp"
#include "client_layer.hpp"

namespace toaster
{
	ClientApplication::ClientApplication() : Application()
	{
		m_layers.push_back(new ClientLayer(this));
	}

	ClientApplication::~ClientApplication()
	{
	}
}
