#pragma once

#include "device.hpp"

namespace toaster::gpu
{
	class TST_GPU_API CommandPool
	{
	public:
		// You are only supposed to create the command pool using the device's function,
		// so I might as well have a default constructor because I hate using 1,000,000 unique ptrs
		CommandPool() = default; // This makes it possible to hash by thread id ;)
		CommandPool(Device& p_device, vk::CommandPool p_pool, ECommandPoolFlags p_flags);
		~CommandPool();

		// Apparently, the compiler cannot properly define default operators that will not completely break the class
		CommandPool(CommandPool&& p_other) noexcept;
		CommandPool& operator=(CommandPool&& p_other) noexcept;

		// I want the class to be move only because it wraps vulkan handles
		CommandPool(const CommandPool&) = delete;
		CommandPool& operator=(const CommandPool&) = delete;

		auto reset() -> void;

		auto createCommandList() -> CommandList;
		auto createCommandLists(uint32 p_count) -> std::vector<CommandList>;

	private:
		auto _destroy() -> void; // Helper thing

		NonOwningPtr<Device> m_device{nullptr};
		vk::CommandPool m_commandPool{nullptr};

		ECommandPoolFlags m_commandPoolFlags{0u};

		friend class CommandList;
	};


}