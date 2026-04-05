#pragma once

#include <vulkan/vulkan_raii.hpp>
#include "toast_lib/core_basic.hpp"

namespace toaster::gpu
{
	class VKGPUContext;

	class VKIndexBuffer
	{
	public:
		VKIndexBuffer(VKGPUContext *p_ctx, void *p_data, uint64 p_size);
		VKIndexBuffer(VKGPUContext *p_ctx, uint64 p_size);

		vk::raii::Buffer &      getBuffer();
		vk::raii::DeviceMemory &getBufferMemory();

		void setData(void *p_data, uint64 p_size, uint64 p_offset);

	private:
		VKGPUContext *m_ctx{nullptr};

		vk::raii::Buffer       m_indexBuffer{nullptr};
		vk::raii::DeviceMemory m_indexBufferMemory{nullptr};
	};
}
