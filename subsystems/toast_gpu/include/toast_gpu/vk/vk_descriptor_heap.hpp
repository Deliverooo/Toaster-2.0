#pragma once

#include "vk_buffer.hpp"
#include "vk_texture.hpp"

namespace toaster::gpu
{
	class VKCommandBuffer;

	using HeapProperties = vk::PhysicalDeviceDescriptorHeapPropertiesEXT; // I ain't writing allat

	using DescriptorSlot = uint32;

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

		auto getBaseBufferAddress() const -> vk::DeviceAddress;
		auto getBaseImageAddress() const -> vk::DeviceAddress;

		auto getSamplerIndex() const -> DescriptorSlot;

		auto allocBuffer(const VKBuffer &p_buffer) -> DescriptorSlot;
		auto allocImage(const VKRawImage &p_image) -> DescriptorSlot;
		auto allocSampler(const vk::SamplerCreateInfo &p_sampler) -> DescriptorSlot;

		auto bind(VKCommandBuffer *p_command_buffer = nullptr) const -> void;

	private:
		HeapProperties m_heapProperties{};

		BufferUnique m_resourceHeap{nullptr};
		BufferUnique m_samplerHeap{nullptr};

		void *m_resourceHeapMemory{nullptr};
		void *m_samplerHeapMemory{nullptr};

		uintptr_t      m_bufferArrayOffset{0u};
		vk::DeviceSize m_bufferArraySize{0u};

		uint32 m_heapResourceIndex{0u};

		uintptr_t      m_imageArrayOffset{0u};
		vk::DeviceSize m_imageArraySize{0u};

		vk::DeviceSize m_samplerHeapSize{0u};
		uint32         m_samplerIndex{0u};
	};

	TST_GPU_DEFINE_HANDLE(VKDescriptorHeap, DescriptorHeap)
}
