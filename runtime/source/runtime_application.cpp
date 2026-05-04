#include "runtime_application.hpp"

#include "runtime_layer.hpp"
#include "toast_gpu/vk/vk_logical_device.hpp"

namespace toaster
{
	RuntimeApplication::RuntimeApplication(const ApplicationCreateInfo &p_create_info, int32 p_argc, char **p_argv) : Application(p_create_info, p_argc, p_argv)
	{
		addLayer(IAppLayer::alloc<RuntimeLayer>(this));
	}
}
