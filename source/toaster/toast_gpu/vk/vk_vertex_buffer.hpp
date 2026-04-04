#pragma once

#include <vulkan/vulkan_raii.hpp>
#include "toast_lib/system_types.h"

namespace toaster::gpu
{
	class VKGPUContext;

	class VKVertexBuffer
	{
	public:
		VKVertexBuffer(VKGPUContext *p_ctx, void *p_data, uint64 p_size);
		VKVertexBuffer(VKGPUContext *p_ctx, uint64 p_size);

		vk::raii::Buffer &      getBuffer();
		vk::raii::DeviceMemory &getBufferMemory();

		void setData(void *p_data, uint64 p_size, uint64 p_offset);

	private:
		VKGPUContext *m_ctx{nullptr};

		vk::raii::Buffer       m_vertexBuffer{nullptr};
		vk::raii::DeviceMemory m_vertexBufferMemory{nullptr};
	};
}
