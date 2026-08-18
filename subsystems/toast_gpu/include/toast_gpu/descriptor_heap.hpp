#pragma once

#include <toast_lib/freelist_allocator.hpp>

#include "allocator.hpp"

namespace toaster::gpu
{
	// Very important note: a descriptor slot with the value of UINT32_MAX is a special value and considered invalid, this makes knowing what to destroy easier.
	using DescriptorSlot = uint32;
	constexpr DescriptorSlot invalidDescriptorSlot{UINT32_MAX};
	constexpr DescriptorSlot invalidBufferDescriptorSlot{invalidDescriptorSlot};
	constexpr DescriptorSlot invalidImageDescriptorSlot{invalidDescriptorSlot};
	constexpr DescriptorSlot invalidSamplerDescriptorSlot{invalidDescriptorSlot};

	class TST_GPU_API ResourceDescriptorHeap
	{
	public:
		ResourceDescriptorHeap(LogicalDevice &p_logical_device, PhysicalDevice &p_physical_device, Allocator &p_allocator, uint32 p_max_buffers, uint32 p_max_images);
		~ResourceDescriptorHeap();

		[[nodiscard]] auto allocBufferSlot() -> DescriptorSlot { return m_bufferSlotAllocator.allocSlot(); }
		[[nodiscard]] auto allocImageSlot() -> DescriptorSlot { return m_imageSlotAllocator.allocSlot(); }
		auto               freeBufferSlot(DescriptorSlot p_slot) -> void { m_bufferSlotAllocator.freeSlot(p_slot); }
		auto               freeImageSlot(DescriptorSlot p_slot) -> void { m_imageSlotAllocator.freeSlot(p_slot); }

		// Basically, to improve performance, when this is called, it will 'queue' the resource to be set.
		auto setBuffer(DescriptorSlot p_slot, const vk::DeviceAddressRangeKHR &p_address_range, vk::DescriptorType p_descriptor_type) -> void;
		auto setImage(DescriptorSlot     p_slot, const vk::ImageViewCreateInfo &p_image_view_create_info, vk::ImageLayout p_image_layout,
					  vk::DescriptorType p_descriptor_type) -> void;

		// When you want to set all your resources, call writeDescriptors() to perform the writeResourceDescriptorsEXT function call
		auto writeDescriptors() -> void;

		auto getBindInfo() const -> const vk::BindHeapInfoEXT & { return m_bindInfo; }

	private:
		NonOwningPtr<LogicalDevice> m_logicalDevice{nullptr};
		NonOwningPtr<Allocator>     m_allocator{nullptr};

		vk::BindHeapInfoEXT m_bindInfo{};

		// The pending queue of resources to set. Will be cleared when writeDescriptors() is called.
		std::vector<vk::HostAddressRangeEXT>       m_bufferHostAddressRanges;
		std::vector<vk::DeviceAddressRangeKHR>     m_bufferDeviceAddressRanges;
		std::vector<vk::ResourceDescriptorInfoEXT> m_bufferResourceInfos;

		std::vector<vk::HostAddressRangeEXT>       m_imageHostAddressRanges;
		std::vector<vk::ImageViewCreateInfo>       m_imageViewCreateInfos;
		std::vector<vk::ImageDescriptorInfoEXT>    m_imageDescriptorInfos;
		std::vector<vk::ResourceDescriptorInfoEXT> m_imageResourceInfos;

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

		[[nodiscard]] auto allocSamplerSlot() -> DescriptorSlot { return m_samplerSlotAllocator.allocSlot(); }
		auto               freeSamplerSlot(DescriptorSlot p_slot) -> void { m_samplerSlotAllocator.freeSlot(p_slot); }

		auto setSampler(DescriptorSlot p_slot, const vk::SamplerCreateInfo &p_sampler_create_info) -> void;

		// When you want to set all your sampler, call writeDescriptors() to perform the writeSamplerDescriptorsEXT function call
		auto writeDescriptors() -> void;

		auto getBindInfo() const -> const vk::BindHeapInfoEXT & { return m_bindInfo; }

	private:
		NonOwningPtr<LogicalDevice> m_logicalDevice{nullptr};
		NonOwningPtr<Allocator>     m_allocator{nullptr};

		vk::BindHeapInfoEXT m_bindInfo{};

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
