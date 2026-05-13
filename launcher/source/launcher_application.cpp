#include "launcher_application.hpp"

#include "launcher_layer.hpp"
#include "toast_gpu/vk/vk_logical_device.hpp"

namespace toaster
{
	LauncherApplication::LauncherApplication(const ApplicationCreateInfo &p_create_info,
											 const CommandLineArgMap &    p_command_line_args) : Application(p_create_info, p_command_line_args)
	{
		addLayer(IAppLayer::alloc<LauncherLayer>(this));
	}
}
