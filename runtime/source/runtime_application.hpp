#pragma once
#include "toaster/toast_kernel/application.hpp"

namespace toaster
{
	class RuntimeApplication : public Application
	{
	public:
		RuntimeApplication(const ApplicationCreateInfo &p_create_info, const CommandLineArgMap &p_command_line_args);
	};
}
