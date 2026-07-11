#include "toast_render/uniform_buffer.hpp"

#include "toast_render/render_context.hpp"

namespace toaster::render
{
	UniformBuffer::UniformBuffer(RenderContext &p_render_ctx, uint64 p_size) : m_renderCtx(&p_render_ctx)
	{
		gpu::BufferSpecInfo ubo_spec_info{};
		ubo_spec_info.usageFlags = vk::BufferUsageFlagBits2::eUniformBuffer | vk::BufferUsageFlagBits2::eShaderDeviceAddressKHR;
		m_ubo                    = m_renderCtx->createGPURef<gpu::Buffer>(p_size, ubo_spec_info);
		m_deviceAddress          = m_ubo->getDeviceAddress();

		m_heapID = m_renderCtx->getDescriptorHeap()->allocBuffer(*m_ubo);
	}

	UniformBuffer::~UniformBuffer()
	{
		m_renderCtx->getDescriptorHeap()->freeBuffer(m_heapID);
	}

	auto UniformBuffer::getDeviceAddress() const -> uintptr
	{
		return m_deviceAddress;
	}

	auto UniformBuffer::getBuffer() const -> const gpu::BufferHandle &
	{
		return m_ubo;
	}

	auto UniformBuffer::getHeapID() const -> gpu::DescriptorSlot
	{
		return m_heapID;
	}

	auto UniformBuffer::getAlignedHeapID() const -> gpu::DescriptorSlot
	{
		return m_heapID + (m_renderCtx->getDescriptorHeap()->getBufferOffset() / m_renderCtx->getDescriptorHeap()->getBufferDescriptorSize());
	}

	UniformBufferPFF::UniformBufferPFF(RenderContext &p_render_ctx, uint64 p_size) : m_renderCtx(&p_render_ctx)
	{
		gpu::BufferSpecInfo ubo_spec_info{};
		ubo_spec_info.usageFlags = vk::BufferUsageFlagBits2::eUniformBuffer | vk::BufferUsageFlagBits2::eShaderDeviceAddressKHR;

		m_ubos.resize(RenderContext::maxFramesInFlight);
		m_heapIDs.resize(RenderContext::maxFramesInFlight);
		m_deviceAddresses.resize(RenderContext::maxFramesInFlight);
		for (uint32 i{0u}; i < RenderContext::maxFramesInFlight; ++i)
		{
			m_ubos[i]            = m_renderCtx->createGPURef<gpu::Buffer>(p_size, ubo_spec_info);
			m_heapIDs[i]         = m_renderCtx->getDescriptorHeap()->allocBuffer(*m_ubos[i]);
			m_deviceAddresses[i] = m_ubos[i]->getDeviceAddress();
		}

		m_mappedData.resize(RenderContext::maxFramesInFlight);
		for (uint32 i{0u}; i < RenderContext::maxFramesInFlight; ++i)
			m_mappedData[i] = m_ubos[i]->mapMemory(p_size, 0u);
	}

	UniformBufferPFF::~UniformBufferPFF()
	{
		for (uint32 i{0u}; i < RenderContext::maxFramesInFlight; ++i)
			m_ubos[i]->unmapMemory();

		for (auto &id: m_heapIDs)
			m_renderCtx->getDescriptorHeap()->freeBuffer(id);
	}

	auto UniformBufferPFF::getDeviceAddress() const -> uintptr
	{
		return m_deviceAddresses[m_renderCtx->getCurrentFrameIndex()];
	}

	auto UniformBufferPFF::getBuffer() const -> const gpu::BufferHandle &
	{
		return m_ubos[m_renderCtx->getCurrentFrameIndex()];
	}

	auto UniformBufferPFF::getHeapID() const -> gpu::DescriptorSlot
	{
		return m_heapIDs[m_renderCtx->getCurrentFrameIndex()];
	}

	auto UniformBufferPFF::getAlignedHeapID() const -> gpu::DescriptorSlot
	{
		return m_heapIDs[m_renderCtx->getCurrentFrameIndex()] + (m_renderCtx->getDescriptorHeap()->getBufferOffset() / m_renderCtx->getDescriptorHeap()->
																 getBufferDescriptorSize());
	}
}
