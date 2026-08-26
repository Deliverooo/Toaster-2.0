#include <toast_gpu/device.hpp>
#include <toast_gpu/material_manager.hpp>
#include <toast_gpu/command_list.hpp>

using namespace toaster;

auto main(TST_UNUSED int32 p_argc, TST_UNUSED char **p_argv) -> int32
{
	gpu::DeviceDesc device_desc{};
	device_desc.enableDebugInfo   = true;
	device_desc.usingSwapchain    = false;
	device_desc.numDeletionQueues = 3u;
	gpu::Device device{device_desc};
	//
	// gpu::CommandList cmd{device.createCommandList()};
	//
	// cmd.begin();
	//
	// cmd.end();
	//
	// device.executeCommandLists({&cmd}, {}, {}, nullptr);
	//
	return 0;
}
