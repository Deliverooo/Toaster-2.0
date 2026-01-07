#pragma once

#include <vulkan/vulkan.hpp>

namespace toaster::gpu
{
	class GPUContext;

	class IndexBuffer
	{
	public:
		IndexBuffer(GPUContext *p_ctx, void *p_data, vk::DeviceSize p_size_bytes);
		~IndexBuffer();

		[[nodiscard]] const vk::Buffer *getBuffer() const { return &m_buffer; }
		vk::DeviceSize                  getSize() const { return m_size; }

	private:
		GPUContext *m_gpuContext;

		vk::DeviceSize   m_size{0u};
		vk::Buffer       m_buffer{nullptr};
		vk::DeviceMemory m_bufferMemory{nullptr};
	};
}
