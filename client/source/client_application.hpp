#pragma once
#include "toaster/toast_kernel/application.hpp"

namespace toaster
{
	class ClientApplication : public Application
	{
	public:
		ClientApplication(const ApplicationCreateInfo &p_create_info, int32 p_argc, char **p_argv);
		~ClientApplication();
	};
}
