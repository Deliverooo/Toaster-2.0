#pragma once

#include "../toast_gpu.hpp"

#include <vulkan/vulkan_raii.hpp>
#include "toast_lib/core_basic.hpp"

namespace toaster::gpu
{
	class TST_GPU_API VKVertexBuffer
	{
		TST_GPU_OBJECT
	public:
		VKVertexBuffer(VKLogicalDevice *p_device, const void *p_data, uint64 p_size);
		VKVertexBuffer(VKLogicalDevice *p_device, uint64 p_size);
		~VKVertexBuffer();

		auto getBuffer() -> vk::Buffer &;
		auto getBufferMemory() -> vk::DeviceMemory &;

		auto setData(void *p_data, uint64 p_size, uint64 p_offset) -> void;

		auto bind(const vk::raii::CommandBuffer &p_command_buffer) -> void;

	private:
		vk::Buffer       m_vertexBuffer{nullptr};
		vk::DeviceMemory m_vertexBufferMemory{nullptr};
	};

	TST_GPU_DEFINE_HANDLE(VKVertexBuffer, VertexBuffer)
}
