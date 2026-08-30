#include <thread>
#include <unordered_map>
#include <toast_gpu/device.hpp>
#include <toast_gpu/material_manager.hpp>
#include <toast_gpu/command_list.hpp>
#include <toast_gpu/command_pool.hpp>

#include <toast_os/console.hpp>
#include <toast_os/entry_points.hpp>

#include "toast_render/mesh.hpp"

using namespace toaster;

TST_WINMAIN()
{
	os::createOutputConsole();
	{
		gpu::DeviceDesc device_desc{};
		device_desc.enableDebugInfo = true;
		device_desc.usingSwapchain = false;
		device_desc.numDeletionQueues = 3u;
		gpu::Device device{device_desc};

		gpu::MaterialManager material_manager{device, 10u * 1028u * 1028u};
		rd::MeshSystem mesh_system{device, material_manager, rd::MeshSystemDesc{}};
	}

	os::destroyOutputConsole();

	return 0;
}
