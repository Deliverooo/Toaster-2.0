#pragma once

#include "vk_buffer.hpp"

namespace toaster::gpu
{
	class VKCommandBuffer;

	using HeapProperties = vk::PhysicalDeviceDescriptorHeapPropertiesEXT; // I ain't writing allat

	class TST_GPU_API VKDescriptorHeap
	{
		TST_GPU_OBJECT
	public:
		VKDescriptorHeap(VKLogicalDevice *p_device);

		[[nodiscard]] auto getHeapProperties() const -> const HeapProperties &;
		[[nodiscard]] auto getResourceHeap() const -> const VKBuffer &;

		auto getNumBuffers() const -> uint32;
		auto getNumImages() const -> uint32;

		auto allocBuffer(const VKBuffer &p_buffer) -> void;

		auto bind(VKCommandBuffer *p_command_buffer = nullptr) const -> void;

	private:
		HeapProperties m_heapProperties{};

		BufferUnique m_resourceHeap{nullptr};

		vk::DeviceSize m_resourceHeapSize{0u};

		uint32 m_numBuffers{0u};
		uint32 m_numImages{0u};
	};

	TST_GPU_DEFINE_HANDLE(VKDescriptorHeap, DescriptorHeap)
}
