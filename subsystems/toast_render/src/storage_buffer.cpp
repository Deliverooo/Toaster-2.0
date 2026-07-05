#include "toast_render/storage_buffer.hpp"

#include "toast_render/render_context.hpp"

namespace toaster::render
{
	StorageBuffer::StorageBuffer(RenderContext &p_render_ctx, uint64 p_size, bool32 p_device_local) : m_renderCtx(&p_render_ctx)
	{
		_construct(p_size, nullptr, p_device_local);
	}

	StorageBuffer::StorageBuffer(RenderContext &p_render_ctx, const void *p_data, uint64 p_size, bool32 p_device_local) : m_renderCtx(&p_render_ctx)

	{
		_construct(p_size, p_data, p_device_local);
	}

	StorageBuffer::~StorageBuffer()
	{
		m_renderCtx->getDescriptorHeap()->freeBuffer(m_heapID);
	}

	auto StorageBuffer::getDeviceAddress() const -> uintptr
	{
		return m_SSBO->getDeviceAddress();
	}

	auto StorageBuffer::getBuffer() const -> const gpu::BufferHandle &
	{
		return m_SSBO;
	}

	auto StorageBuffer::getHeapID() const -> gpu::DescriptorSlot
	{
		return m_heapID;
	}

	auto StorageBuffer::getAlignedHeapID() const -> gpu::DescriptorSlot
	{
		return m_heapID + (m_renderCtx->getDescriptorHeap()->getBufferOffset() / m_renderCtx->getDescriptorHeap()->getBufferDescriptorSize());
	}

	auto StorageBuffer::setData(const void *p_data, uint64 p_size) -> void
	{
		m_SSBO->setData(p_data, p_size);
	}

	auto StorageBuffer::_construct(uint64 p_size, const void *p_data, bool32 p_device_local) -> void
	{
		if (!p_device_local)
		{
			gpu::BufferSpecInfo ubo_spec_info{};
			ubo_spec_info.usageFlags = vk::BufferUsageFlagBits2::eStorageBuffer;
			m_SSBO                   = m_renderCtx->createGPURef<gpu::Buffer>(p_size, ubo_spec_info);

			if (p_data)
				m_SSBO->setData(p_data, p_size);
		}
		else
		{
			gpu::BufferSpecInfo ubo_spec_info{};
			ubo_spec_info.deviceLocal = true;
			ubo_spec_info.usageFlags  = vk::BufferUsageFlagBits2::eStorageBuffer | vk::BufferUsageFlagBits2::eTransferDst;
			m_SSBO                    = m_renderCtx->createGPURef<gpu::Buffer>(p_size, ubo_spec_info);

			if (p_data)
			{
				gpu::BufferSpecInfo staging_buffer_spec_info{};
				staging_buffer_spec_info.deviceLocal = false;
				staging_buffer_spec_info.usageFlags  = vk::BufferUsageFlagBits2::eTransferSrc;
				gpu::Buffer staging_buffer{m_renderCtx->getLogicalDevice(), p_size, staging_buffer_spec_info};
				staging_buffer.setData(p_data, p_size);

				m_SSBO->copyFromBuffer(staging_buffer);
			}
		}

		m_heapID = m_renderCtx->getDescriptorHeap()->allocBuffer(*m_SSBO, true);
	}

	StorageBufferPFF::StorageBufferPFF(RenderContext &p_render_ctx, uint64 p_size) : m_renderCtx(&p_render_ctx)
	{
		gpu::BufferSpecInfo ubo_spec_info{};
		ubo_spec_info.usageFlags = vk::BufferUsageFlagBits2::eStorageBuffer | vk::BufferUsageFlagBits2::eShaderDeviceAddressKHR;

		m_ssbos.resize(RenderContext::maxFramesInFlight);
		m_heapIDs.resize(RenderContext::maxFramesInFlight);
		for (uint32 i{0u}; i < RenderContext::maxFramesInFlight; ++i)
		{
			m_ssbos[i]   = m_renderCtx->createGPURef<gpu::Buffer>(p_size, ubo_spec_info);
			m_heapIDs[i] = m_renderCtx->getDescriptorHeap()->allocBuffer(*m_ssbos[i], true);
		}

		m_mappedData.resize(RenderContext::maxFramesInFlight);
		for (uint32 i{0u}; i < RenderContext::maxFramesInFlight; ++i)
			m_mappedData[i] = m_ssbos[i]->mapMemory(p_size, 0u);
	}

	StorageBufferPFF::~StorageBufferPFF()
	{
		for (uint32 i{0u}; i < RenderContext::maxFramesInFlight; ++i)
			m_ssbos[i]->unmapMemory();

		for (auto &id: m_heapIDs)
			m_renderCtx->getDescriptorHeap()->freeBuffer(id);
	}

	auto StorageBufferPFF::getDeviceAddress() const -> uintptr
	{
		return m_ssbos[m_renderCtx->getCurrentFrameIndex()]->getDeviceAddress();
	}

	auto StorageBufferPFF::getBuffer() const -> const gpu::BufferHandle &
	{
		return m_ssbos[m_renderCtx->getCurrentFrameIndex()];
	}

	auto StorageBufferPFF::getHeapID() const -> gpu::DescriptorSlot
	{
		return m_heapIDs[m_renderCtx->getCurrentFrameIndex()];
	}

	auto StorageBufferPFF::getAlignedHeapID() const -> gpu::DescriptorSlot
	{
		return m_heapIDs[m_renderCtx->getCurrentFrameIndex()] + (m_renderCtx->getDescriptorHeap()->getBufferOffset() / m_renderCtx->getDescriptorHeap()->
																 getBufferDescriptorSize());
	}
}
