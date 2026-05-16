#include "runtime_application.hpp"

#include "runtime_layer.hpp"
#include "toast_gpu/vk/vk_logical_device.hpp"

namespace toaster
{
	RuntimeApplication::RuntimeApplication(const ApplicationCreateInfo &p_create_info,
										   const CommandLineArgs *      p_command_line_args) : Application(p_create_info, p_command_line_args)
	{
		addLayer(IAppLayer::alloc<RuntimeLayer>(this));
	}
}
