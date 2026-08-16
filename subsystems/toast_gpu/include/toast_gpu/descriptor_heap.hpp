#pragma once

#include <toast_lib/freelist_allocator.hpp>

#include "allocator.hpp"

namespace toaster::gpu
{
	class TST_GPU_API ResourceDescriptorHeap
	{
	public:
		ResourceDescriptorHeap(LogicalDevice &p_logical_device, PhysicalDevice &p_physical_device, Allocator &p_allocator, uint32 p_max_buffers, uint32 p_max_images);
		~ResourceDescriptorHeap();

		// Basically, to improve performance, when this is called, it will 'queue' the resource to be set.
		auto setBuffer(uint32 p_slot, const vk::DeviceAddressRangeKHR &p_address_range, vk::DescriptorType p_descriptor_type) -> void;
		auto setImage(uint32             p_slot, const vk::ImageViewCreateInfo &p_image_view_create_info, vk::ImageLayout p_image_layout,
					  vk::DescriptorType p_descriptor_type) -> void;

		// When you want to set all your resources, call writeDescriptors() to perform the writeResourceDescriptorsEXT function call
		auto writeDescriptors() -> void;

	private:
		NonOwningPtr<LogicalDevice> m_logicalDevice{nullptr};
		NonOwningPtr<Allocator>     m_allocator{nullptr};

		// The pending queue of resources to set. Will be cleared when writeDescriptors() is called.
		std::vector<vk::HostAddressRangeEXT>       m_hostAddressRanges;
		std::vector<vk::ResourceDescriptorInfoEXT> m_resourceInfos;

		vk::Buffer    m_heapBuffer{nullptr};
		VmaAllocation m_heapAllocation{nullptr};
		void *        m_mappedHeapMemory{nullptr};

		FreelistAllocator<uint32> m_bufferSlotAllocator;
		FreelistAllocator<uint32> m_imageSlotAllocator;

		vk::DeviceSize m_bufferDescriptorSize{0u};
		vk::DeviceSize m_imageDescriptorSize{0u};

		vk::DeviceSize m_bufferSegmentSize{0u};

		uint64         m_imageSegmentOffset{0u};
		vk::DeviceSize m_imageSegmentSize{0u};
	};

	class TST_GPU_API SamplerDescriptorHeap
	{
	public:
		SamplerDescriptorHeap(LogicalDevice &p_logical_device, PhysicalDevice &p_physical_device, Allocator &p_allocator, uint32 p_max_samplers);
		~SamplerDescriptorHeap();

		auto setSampler(uint32 p_slot, const vk::SamplerCreateInfo &p_sampler_create_info) -> void;

		// When you want to set all your sampler, call writeDescriptors() to perform the writeSamplerDescriptorsEXT function call
		auto writeDescriptors() -> void;

	private:
		NonOwningPtr<LogicalDevice> m_logicalDevice{nullptr};
		NonOwningPtr<Allocator>     m_allocator{nullptr};

		// The pending queue of samplers to set. Will be cleared when writeDescriptors() is called.
		std::vector<vk::HostAddressRangeEXT> m_hostAddressRanges;
		std::vector<vk::SamplerCreateInfo>   m_samplerCreateInfos;

		vk::Buffer    m_heapBuffer{nullptr};
		VmaAllocation m_heapAllocation{nullptr};
		void *        m_mappedHeapMemory{nullptr};

		FreelistAllocator<uint32> m_samplerSlotAllocator;

		vk::DeviceSize m_samplerDescriptorSize{0u};
	};
}
