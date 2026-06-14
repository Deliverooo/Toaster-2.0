#pragma once

#include "render_context.hpp"
#include "toast_render.hpp"
#include "toast_gpu/vk/vk_buffer.hpp"
#include "toast_gpu/vk/vk_descriptor_heap.hpp"

namespace toaster::render
{
	class RenderContext;

	class TST_RENDER_API UniformBuffer
	{
		TST_RENDER_OBJECT
	public:
		UniformBuffer(RenderContext &p_render_ctx, uint64 p_size);
		~UniformBuffer();

		auto getDeviceAddress() const -> uintptr;
		auto getBuffer() const -> const gpu::BufferHandle &;

		auto getHeapID() const -> gpu::DescriptorSlot;
		auto getAlignedHeapID() const -> gpu::DescriptorSlot;

		template<typename Type>
		auto setData(const Type &p_data) -> void
		{
			m_ubo->setData(p_data);
		}

	private:
		gpu::BufferHandle   m_ubo{nullptr};
		gpu::DescriptorSlot m_heapID{UINT32_MAX};
	};

	TST_RENDER_DEFINE_HANDLE(UniformBuffer, UniformBuffer);

	class TST_RENDER_API UniformBufferPFF
	{
		TST_RENDER_OBJECT
	public:
		UniformBufferPFF(RenderContext &p_render_ctx, uint64 p_size);
		~UniformBufferPFF();

		auto getDeviceAddress() const -> uintptr;
		auto getBuffer() const -> const gpu::BufferHandle &;

		auto getHeapID() const -> gpu::DescriptorSlot;
		auto getAlignedHeapID() const -> gpu::DescriptorSlot;

		template<typename Type>
		auto setData(const Type &p_data) -> void
		{
			std::memcpy(m_mappedData[m_renderCtx->getCurrentFrameIndex()], &p_data, sizeof(Type));
		}

		template<typename Type>
		auto setAllData(const Type &p_data) -> void
		{
			for (auto &data: m_mappedData)
				std::memcpy(data, &p_data, sizeof(Type));
		}

	private:
		gpu::PerFrameVec<gpu::BufferHandle>   m_ubos;
		gpu::PerFrameVec<gpu::DescriptorSlot> m_heapIDs;

		gpu::PerFrameVec<void *> m_mappedData;
	};

	TST_RENDER_DEFINE_HANDLE(UniformBufferPFF, UniformBufferPFF);
}
