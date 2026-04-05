#pragma once

#include <vulkan/vulkan_raii.hpp>
#include "toast_lib/system_types.h"

namespace toaster::gpu
{
	class VKGPUContext;

	class VKUniformBuffer
	{
	public:
		VKUniformBuffer(VKGPUContext *p_ctx, uint64 p_size);

		vk::raii::Buffer &      getBuffer();
		vk::raii::DeviceMemory &getBufferMemory();

		const vk::DescriptorBufferInfo &getDescriptorInfo() const;

		void setData(void *p_data, uint64 p_size, uint64 p_offset);

		void *mapMemory(uint64 p_size, uint64 p_offset);
		void  unmapMemory();

	private:
		VKGPUContext *m_ctx{nullptr};

		vk::raii::Buffer         m_buffer{nullptr};
		vk::raii::DeviceMemory   m_bufferMemory{nullptr};
		vk::DescriptorBufferInfo m_descriptorInfo{};
	};
}
