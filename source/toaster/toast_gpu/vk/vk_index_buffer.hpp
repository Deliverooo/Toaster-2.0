#pragma once

#include "../toast_gpu.hpp"

#include <vulkan/vulkan_raii.hpp>
#include "toast_lib/core_basic.hpp"

namespace toaster::gpu
{
	class VKLogicalDevice;

	class TST_GPU_API VKIndexBuffer
	{
	public:
		// Note to self, always remember to upload the indices instead of just passing in the size...
		// I'm such a diddyblud
		VKIndexBuffer(VKLogicalDevice *p_device, void *p_data, uint64 p_size);
		VKIndexBuffer(VKLogicalDevice *p_device, uint64 p_size);
		[[nodiscard]] auto getDevice() const -> VKLogicalDevice *;


		auto getBuffer() -> vk::raii::Buffer &;
		auto getBufferMemory() -> vk::raii::DeviceMemory &;

		auto setData(void *p_data, uint64 p_size, uint64 p_offset) -> void;

		auto bind(const vk::raii::CommandBuffer &p_command_buffer, vk::IndexType p_index_type) -> void;

	private:
		VKLogicalDevice *m_device{nullptr};

		vk::raii::Buffer       m_indexBuffer{nullptr};
		vk::raii::DeviceMemory m_indexBufferMemory{nullptr};
	};
}
