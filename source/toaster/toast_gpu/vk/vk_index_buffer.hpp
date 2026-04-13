#pragma once

#include <vulkan/vulkan_raii.hpp>
#include "toast_lib/core_basic.hpp"

namespace toaster::gpu
{
	class VKGPUContext;

	class VKIndexBuffer
	{
	public:
		// Note to self, always remember to upload the indices instead of just passing in the size...
		// I'm such a diddyblud
		VKIndexBuffer(VKGPUContext *p_ctx, void *p_data, uint64 p_size);
		VKIndexBuffer(VKGPUContext *p_ctx, uint64 p_size);
		auto getContext() const -> VKGPUContext *;

		auto getBuffer() -> vk::raii::Buffer &;
		auto getBufferMemory() -> vk::raii::DeviceMemory &;

		auto setData(void *p_data, uint64 p_size, uint64 p_offset) -> void;

		auto bind(const vk::raii::CommandBuffer &p_command_buffer, vk::IndexType p_index_type) -> void;

	private:
		VKGPUContext *m_ctx{nullptr};

		vk::raii::Buffer       m_indexBuffer{nullptr};
		vk::raii::DeviceMemory m_indexBufferMemory{nullptr};
	};
}
