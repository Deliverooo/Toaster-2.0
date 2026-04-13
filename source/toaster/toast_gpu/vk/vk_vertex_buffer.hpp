#pragma once

#include <vulkan/vulkan_raii.hpp>
#include "toast_lib/core_basic.hpp"

namespace toaster::gpu
{
	class VKGPUContext;

	class VKVertexBuffer
	{
	public:
		VKVertexBuffer(VKGPUContext *p_ctx, void *p_data, uint64 p_size);
		VKVertexBuffer(VKGPUContext *p_ctx, uint64 p_size);
		auto getContext() const -> VKGPUContext *;

		auto getBuffer() -> vk::raii::Buffer &;
		auto getBufferMemory() -> vk::raii::DeviceMemory &;

		auto setData(void *p_data, uint64 p_size, uint64 p_offset) -> void;

		auto bind(const vk::raii::CommandBuffer &p_command_buffer) -> void;

	private:
		VKGPUContext *m_ctx{nullptr};

		vk::raii::Buffer       m_vertexBuffer{nullptr};
		vk::raii::DeviceMemory m_vertexBufferMemory{nullptr};
	};
}
