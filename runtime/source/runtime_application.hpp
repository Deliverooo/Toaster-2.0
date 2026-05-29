#pragma once

#include "toast_kernel/application.hpp"

namespace toaster
{
	class RuntimeApplication : public Application
	{
	public:
		RuntimeApplication(const ApplicationSpecInfo &p_create_info, const CommandLineArgs *p_command_line_args);
	};
}
