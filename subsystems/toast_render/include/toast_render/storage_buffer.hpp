#pragma once

#include "render_context.hpp"
#include "toast_render.hpp"
#include "toast_gpu/vk/vk_buffer.hpp"
#include "toast_gpu/vk/vk_descriptor_heap.hpp"

namespace toaster::render
{
	class RenderContext;

	class TST_RENDER_API StorageBuffer
	{
		TST_RENDER_OBJECT
	public:
		template<typename Type>
		StorageBuffer(RenderContext &p_render_ctx, const Type &p_data, bool32 p_device_local = false) : m_renderCtx(&p_render_ctx)
		{
			_construct(sizeof(Type), (const void *) &p_data, p_device_local);
		}

		template<typename Type>
		StorageBuffer(RenderContext &p_render_ctx, const std::vector<Type> &p_data, bool32 p_device_local = false) : m_renderCtx(&p_render_ctx)
		{
			_construct(p_data.size() * sizeof(Type), p_data.data(), p_device_local);
		}

		StorageBuffer(RenderContext &p_render_ctx, uint64 p_size, bool32 p_device_local = false);
		StorageBuffer(RenderContext &p_render_ctx, const void *p_data, uint64 p_size, bool32 p_device_local = false);
		~StorageBuffer();

		auto getDeviceAddress() const -> uintptr;
		auto getBuffer() const -> const gpu::BufferHandle &;

		// These do not work at all because vk_Ext_descriptor_heap is so new, GLSL and HLSL don't have proper support for buffer indexing. Use BDA instead!
		auto getHeapID() const -> gpu::DescriptorSlot;
		auto getAlignedHeapID() const -> gpu::DescriptorSlot;

		auto setData(const void *p_data, uint64 p_size) -> void;

		template<typename Type>
		auto setData(const Type &p_data) -> void
		{
			setData(&p_data, sizeof(Type));
		}

		template<typename Type>
		auto setData(const std::vector<Type> &p_data) -> void
		{
			setData(p_data.data(), p_data.size() * sizeof(Type));
		}

	private:
		auto _construct(uint64 p_size, const void *p_data = nullptr, bool32 p_device_local = false) -> void;

		gpu::BufferHandle m_SSBO{nullptr};

		gpu::DescriptorSlot m_heapID{UINT32_MAX};
	};

	TST_RENDER_DEFINE_HANDLE(StorageBuffer, StorageBuffer);

	using VertexBuffer = StorageBuffer;
	using IndexBuffer  = StorageBuffer; // Technically I am still waiting on the driver update that will let me use storage buffers as index buffers (24/06/26)
	TST_RENDER_DEFINE_HANDLE(VertexBuffer, VertexBuffer);
	TST_RENDER_DEFINE_HANDLE(IndexBuffer, IndexBuffer);

	class TST_RENDER_API StorageBufferPFF
	{
		TST_RENDER_OBJECT
	public:
		StorageBufferPFF(RenderContext &p_render_ctx, uint64 p_size);
		~StorageBufferPFF();

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
		gpu::PerFrameVec<gpu::BufferHandle>   m_ssbos;
		gpu::PerFrameVec<gpu::DescriptorSlot> m_heapIDs;

		gpu::PerFrameVec<void *> m_mappedData;
	};

	TST_RENDER_DEFINE_HANDLE(StorageBufferPFF, StorageBufferPFF);
}
