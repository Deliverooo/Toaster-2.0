#include <thread>
#include <unordered_map>
#include <toast_gpu/device.hpp>
#include <toast_gpu/material_manager.hpp>
#include <toast_gpu/command_list.hpp>
#include <toast_gpu/command_pool.hpp>

#include <toast_os/console.hpp>
#include <toast_os/entry_points.hpp>

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

		std::unordered_map<std::thread::id, gpu::CommandPool> thread_pools;

		auto &command_pool{thread_pools[std::this_thread::get_id()]};
		command_pool = device.createCommandPool(gpu::EQueueType::eGraphics, gpu::ECommandPoolBits::eReset);

		gpu::CommandList cmd{command_pool.createCommandList()};

		cmd.begin();

		cmd.end();

		device.executeCommandLists(cmd);
		device.waitIdle();

		command_pool.reset();

	}

	os::destroyOutputConsole();

	return 0;
}
