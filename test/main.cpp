#include <thread>
#include <unordered_map>
#include <toast_gpu/device.hpp>
#include <toast_gpu/command_list.hpp>
#include <toast_gpu/command_pool.hpp>

#include <toast_os/console.hpp>
#include <toast_os/entry_points.hpp>

#include "toast_render/mesh_system.hpp"

using namespace toaster;

TST_WINMAIN()
{
	os::createOutputConsole();
	{
		gpu::DeviceDesc device_desc{};
		device_desc.enableDebugInfo   = true;
		device_desc.usingSwapchain    = false;
		device_desc.numDeletionQueues = 3u;
		gpu::Device device{device_desc};

		gpu::ResourceManager resource_manager{device, gpu::ResourceManagerDesc{}};

		gpu::BufferHandle buffer{resource_manager.createBuffer(gpu::BufferDesc::staging(sizeof(uint32)))};

		resource_manager.destroyBuffer(buffer);
	}

	os::destroyOutputConsole();

	return 0;
}
