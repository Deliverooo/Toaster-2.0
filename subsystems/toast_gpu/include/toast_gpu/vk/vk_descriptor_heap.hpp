#pragma once

#include "vk_buffer.hpp"
#include "vk_texture.hpp"

namespace toaster::gpu
{
	class VKCommandBuffer;

	using HeapProperties = vk::PhysicalDeviceDescriptorHeapPropertiesEXT; // I ain't writing allat

	using DescriptorSlot = uint32;

	class DescriptorSlotManager
	{
	public:
		DescriptorSlotManager() = default;
		DescriptorSlotManager(uint32 p_capacity);

		[[nodiscard]] auto allocSlot() -> DescriptorSlot;
		auto               freeSlot(DescriptorSlot p_slot) -> void;

	private:
		std::vector<DescriptorSlot> m_freeSlots;
	};

	class TST_GPU_API VKDescriptorHeap
	{
		TST_GPU_OBJECT
	public:
		static constexpr uint32 maxUBOs{24u};
		static constexpr uint32 maxImages{24u};

		static constexpr uint32 maxSamplers{24u};

		VKDescriptorHeap(VKLogicalDevice *p_device);
		~VKDescriptorHeap();

		[[nodiscard]] auto getHeapProperties() const -> const HeapProperties &;
		[[nodiscard]] auto getResourceHeap() const -> const VKBuffer &;
		[[nodiscard]] auto getSamplerHeap() const -> const VKBuffer &;

		auto getBufferDescriptorSize() const -> vk::DeviceSize;
		auto getImageDescriptorSize() const -> vk::DeviceSize;

		auto getBufferOffset() const -> uintptr_t;
		auto getImageOffset() const -> uintptr_t;

		auto allocBuffer(const Buffer &p_buffer) -> DescriptorSlot;
		auto allocImage(const RawImage &p_image) -> DescriptorSlot;
		auto allocSampler(const vk::SamplerCreateInfo &p_sampler) -> DescriptorSlot;

		auto setBuffer(DescriptorSlot p_slot, const Buffer &p_buffer) -> void;
		auto setImage(DescriptorSlot p_slot, const RawImage &p_image) -> void;
		auto setSampler(DescriptorSlot p_slot, const vk::SamplerCreateInfo &p_sampler) -> void;

		auto getOffset(DescriptorSlot p_slot) -> uint64;

		auto freeBuffer(DescriptorSlot p_slot) -> void;
		auto freeImage(DescriptorSlot p_slot) -> void;
		auto freeSampler(DescriptorSlot p_slot) -> void;

		auto bind(VKCommandBuffer *p_command_buffer = nullptr) const -> void;

	private:
		HeapProperties m_heapProperties{};

		BufferUnique m_resourceHeap{nullptr};
		BufferUnique m_samplerHeap{nullptr};

		void *m_resourceHeapMemory{nullptr};
		void *m_samplerHeapMemory{nullptr};

		vk::DeviceSize m_bufferDescriptorSize{0u};
		vk::DeviceSize m_imageDescriptorSize{0u};

		uintptr_t      m_bufferArrayOffset{0u};
		vk::DeviceSize m_bufferArraySize{0u};

		uintptr_t      m_imageArrayOffset{0u};
		vk::DeviceSize m_imageArraySize{0u};

		DescriptorSlotManager m_bufferSlotManager;
		DescriptorSlotManager m_imageSlotManager;

		DescriptorSlotManager m_samplerSlotManager;
	};

	TST_GPU_DEFINE_HANDLE(VKDescriptorHeap, DescriptorHeap)
}
