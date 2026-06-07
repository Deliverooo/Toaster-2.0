#include "toast_render/uniform_buffer.hpp"

#include "toast_render/render_context.hpp"

namespace toaster::render
{
	UniformBuffer::UniformBuffer(RenderContext &p_render_ctx, uint64 p_size) : m_renderCtx(&p_render_ctx)
	{
		gpu::BufferSpecInfo ubo_spec_info{};
		ubo_spec_info.usageFlags = vk::BufferUsageFlagBits2::eUniformBuffer | vk::BufferUsageFlagBits2::eShaderDeviceAddressKHR;
		m_ubo                    = m_renderCtx->createGPURef<gpu::Buffer>(p_size, ubo_spec_info);

		m_heapID = m_renderCtx->getDescriptorHeap()->allocBuffer(*m_ubo);
	}

	UniformBuffer::~UniformBuffer()
	{
		m_renderCtx->getDescriptorHeap()->freeBuffer(m_heapID);
	}

	auto UniformBuffer::getDeviceAddress() const -> uintptr
	{
		return m_ubo->getDeviceAddress();
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
		return m_heapID + (m_renderCtx->getDescriptorHeap()->getBufferOffset() / m_renderCtx->getDescriptorHeap()->getHeapProperties().bufferDescriptorSize);
	}

	UniformBufferPFF::UniformBufferPFF(RenderContext &p_render_ctx, uint64 p_size) : m_renderCtx(&p_render_ctx)
	{
		gpu::BufferSpecInfo ubo_spec_info{};
		ubo_spec_info.usageFlags = vk::BufferUsageFlagBits2::eUniformBuffer | vk::BufferUsageFlagBits2::eShaderDeviceAddressKHR;

		m_ubos.resize(RenderContext::maxFramesInFlight);
		m_heapIDs.resize(RenderContext::maxFramesInFlight);
		for (uint32 i{0u}; i < RenderContext::maxFramesInFlight; ++i)
		{
			m_ubos[i]    = m_renderCtx->createGPURef<gpu::Buffer>(p_size, ubo_spec_info);
			m_heapIDs[i] = m_renderCtx->getDescriptorHeap()->allocBuffer(*m_ubos[i]);
		}
	}

	UniformBufferPFF::~UniformBufferPFF()
	{
		for (auto &id: m_heapIDs)
			m_renderCtx->getDescriptorHeap()->freeBuffer(id);
	}

	auto UniformBufferPFF::getDeviceAddress() const -> uintptr
	{
		return m_ubos[m_renderCtx->getCurrentFrameIndex()]->getDeviceAddress();
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
																 getHeapProperties().bufferDescriptorSize);
	}
}
