#include "index_buffer.hpp"
#include "gpu_context.hpp"

namespace toaster::gpu
{
	IndexBuffer::IndexBuffer(GPUContext *p_ctx, void *p_data, vk::DeviceSize p_size_bytes) : m_gpuContext(p_ctx)
	{
		const vk::Device device = m_gpuContext->getLogicalDevice();

		// Create staging buffer
		vk::Buffer       stagingBuffer;
		vk::DeviceMemory stagingMemory;
		// m_gpuContext->createBuffer(p_size_bytes, vk::BufferUsageFlagBits::eTransferSrc,
								   // vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingMemory);

		// Copy index data to staging buffer
		void *data = nullptr;
		vk::detail::resultCheck(device.mapMemory(stagingMemory, 0, p_size_bytes, static_cast<vk::MemoryMapFlagBits>(0), &data),
								"Failed to map index staging buffer memory");
		std::memcpy(data, p_data, p_size_bytes);
		device.unmapMemory(stagingMemory);

		// Create device-local index buffer
		// m_gpuContext->createBuffer(p_size_bytes, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal,
								   // m_buffer, m_bufferMemory);

		// Copy from staging to device-local buffer
		// m_gpuContext->copyBuffer(stagingBuffer, m_buffer, p_size_bytes);

		// Clean up staging buffer
		device.destroyBuffer(stagingBuffer);
		device.freeMemory(stagingMemory);
	}

	IndexBuffer::~IndexBuffer()
	{
		const vk::Device device = m_gpuContext->getLogicalDevice();

		device.destroyBuffer(m_buffer);
		device.freeMemory(m_bufferMemory);
	}

}
