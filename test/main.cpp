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

		rd::MaterialSystem material_system{device, 10u * 1028u * 1028u};
		rd::MeshSystem     mesh_system{device, material_system, rd::MeshSystemDesc{}};

		Ref<gpu::Device, gpu::BufferHandle> buffer{device.createBuffer(gpu::BufferDesc::staging(sizeof(uint32)))};
	}

	os::destroyOutputConsole();

	return 0;
}
