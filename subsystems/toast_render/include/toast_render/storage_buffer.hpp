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
		StorageBuffer(RenderContext &p_render_ctx, const Type &p_data, const String &p_debug_name = "Storage_buffer") : m_renderCtx(&p_render_ctx),
																														m_debugName(p_debug_name)
		{
			_construct(sizeof(Type), (const void *) &p_data);
		}

		template<typename Type>
		StorageBuffer(RenderContext &p_render_ctx, const std::vector<Type> &p_data, const String &p_debug_name = "Storage_buffer") : m_renderCtx(&p_render_ctx),
																																	 m_debugName(p_debug_name)
		{
			_construct(p_data.size() * sizeof(Type), p_data.data());
		}

		StorageBuffer(RenderContext &p_render_ctx, uint64 p_size, const String &p_debug_name = "Storage_buffer");
		StorageBuffer(RenderContext &p_render_ctx, const void *p_data, uint64 p_size, const String &p_debug_name = "Storage_buffer");
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
		auto _construct(uint64 p_size, const void *p_data = nullptr) -> void;

		String m_debugName;

		gpu::BufferHandle m_SSBO{nullptr};

		gpu::DescriptorSlot m_heapID{UINT32_MAX};
	};

	TST_RENDER_DEFINE_HANDLE(StorageBuffer, StorageBuffer);

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
