#include <toast_gpu/device.hpp>
#include <toast_gpu/material_manager.hpp>
#include <toast_gpu/command_list.hpp>

#include "toast_gpu/mesh_manager.hpp"

using namespace toaster;

auto main(TST_UNUSED int32 p_argc, TST_UNUSED char **p_argv) -> int32
{
	gpu::DeviceDesc device_desc{};
	device_desc.enableDebugInfo = true;
	device_desc.usingSwapchain = false;
	device_desc.numDeletionQueues = 3u;
	gpu::Device device{device_desc};

	{
		gpu::MaterialManager material_manager{device, 10u * 1028u * 1028u};

		gpu::MeshManager mesh_manager{
			device, material_manager, sizeof(gpu::StaticMeshVertex) * 10u * 1028u * 1028u,
			sizeof(uint32) * 10u * 1028u * 1028u
		};


	}
	device.flushDeletionQueue();

	return 0;
}
