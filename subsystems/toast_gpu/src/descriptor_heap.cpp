#include "toast_gpu/descriptor_heap.hpp"

namespace toaster::gpu
{
	ResourceDescriptorHeap::ResourceDescriptorHeap(LogicalDevice &p_logical_device, PhysicalDevice &p_physical_device, Allocator &p_allocator, uint32 p_max_buffers,
												   uint32         p_max_images) : m_logicalDevice(&p_logical_device), m_allocator(&p_allocator)
	{
		m_bufferSlotAllocator = {p_max_buffers};
		m_imageSlotAllocator  = {p_max_buffers};

		const auto &heap_props{p_physical_device.getDescriptorHeapProperties()};

		m_bufferDescriptorSize = TST_ALIGN(heap_props.bufferDescriptorSize, heap_props.bufferDescriptorAlignment);
		m_imageDescriptorSize  = TST_ALIGN(heap_props.imageDescriptorSize, heap_props.imageDescriptorAlignment);

		m_bufferSegmentSize = (p_max_buffers * m_bufferDescriptorSize);

		m_imageSegmentOffset = TST_ALIGN(m_bufferSegmentSize, heap_props.imageDescriptorAlignment);
		m_imageSegmentSize   = (p_max_images * m_imageDescriptorSize);

		vk::DeviceSize resource_heap_size{
			TST_ALIGN(m_bufferSegmentSize + m_imageSegmentSize + heap_props.minResourceHeapReservedRange, heap_props.resourceHeapAlignment)
		};

		vk::BufferCreateInfo resource_heap_create_info{};
		resource_heap_create_info.usage       = vk::BufferUsageFlagBits::eDescriptorHeapEXT | vk::BufferUsageFlagBits::eShaderDeviceAddressKHR;
		resource_heap_create_info.sharingMode = vk::SharingMode::eExclusive;
		resource_heap_create_info.size        = resource_heap_size;
		resource_heap_create_info.setQueueFamilyIndices(p_logical_device.getQueueFamilyIndices().graphics);

		VmaAllocationCreateInfo heap_allocation_create_info{};
		heap_allocation_create_info.usage = VMA_MEMORY_USAGE_AUTO;
		heap_allocation_create_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

		vmaCreateBuffer(m_allocator->getAllocator(), reinterpret_cast<VkBufferCreateInfo *>(&resource_heap_create_info), &heap_allocation_create_info,
						reinterpret_cast<VkBuffer *>(&m_heapBuffer), &m_heapAllocation, nullptr);

		vmaMapMemory(m_allocator->getAllocator(), m_heapAllocation, &m_mappedHeapMemory);
	}

	ResourceDescriptorHeap::~ResourceDescriptorHeap()
	{
		vmaUnmapMemory(m_allocator->getAllocator(), m_heapAllocation);
		vmaDestroyBuffer(m_allocator->getAllocator(), m_heapBuffer, m_heapAllocation);
	}

	auto ResourceDescriptorHeap::setBuffer(uint32 p_slot, const vk::DeviceAddressRangeKHR &p_address_range, vk::DescriptorType p_descriptor_type) -> void
	{
		vk::HostAddressRangeEXT &host_range{m_hostAddressRanges.emplace_back()};
		host_range.address = reinterpret_cast<void *>(reinterpret_cast<uint64>(m_mappedHeapMemory) + static_cast<uint64>(p_slot) * m_bufferDescriptorSize);
		host_range.size    = m_bufferDescriptorSize;

		vk::ResourceDescriptorInfoEXT &resource_info{m_resourceInfos.emplace_back()};
		resource_info.type               = p_descriptor_type;
		resource_info.data.pAddressRange = &p_address_range;
	}

	auto ResourceDescriptorHeap::setImage(uint32             p_slot, const vk::ImageViewCreateInfo &p_image_view_create_info, vk::ImageLayout p_image_layout,
										  vk::DescriptorType p_descriptor_type) -> void
	{
		vk::HostAddressRangeEXT &host_range{m_hostAddressRanges.emplace_back()};
		host_range.address = reinterpret_cast<void *>(reinterpret_cast<uint64>(m_mappedHeapMemory) + m_imageSegmentOffset + static_cast<uint64>(p_slot) *
													  m_imageDescriptorSize);
		host_range.size = m_imageDescriptorSize;

		vk::ImageDescriptorInfoEXT image_info{};
		image_info.pView  = &p_image_view_create_info;
		image_info.layout = p_image_layout;

		vk::ResourceDescriptorInfoEXT &resource_info{m_resourceInfos.emplace_back()};
		resource_info.type        = p_descriptor_type;
		resource_info.data.pImage = &image_info;
	}

	auto ResourceDescriptorHeap::writeDescriptors() -> void
	{
		m_logicalDevice->getDevice().writeResourceDescriptorsEXT(m_resourceInfos, m_hostAddressRanges);
		m_resourceInfos.clear();
		m_hostAddressRanges.clear();
	}

	SamplerDescriptorHeap::SamplerDescriptorHeap(LogicalDevice &p_logical_device, PhysicalDevice &p_physical_device, Allocator &p_allocator,
												 uint32         p_max_samplers) : m_logicalDevice(&p_logical_device), m_allocator(&p_allocator)
	{
		m_samplerSlotAllocator = {p_max_samplers};

		const auto &heap_props{p_physical_device.getDescriptorHeapProperties()};

		m_samplerDescriptorSize = TST_ALIGN(heap_props.samplerDescriptorSize, heap_props.samplerDescriptorAlignment);

		vk::DeviceSize sampler_heap_size{TST_ALIGN(p_max_samplers * m_samplerDescriptorSize + heap_props.minResourceHeapReservedRange, heap_props.resourceHeapAlignment)};

		vk::BufferCreateInfo sampler_heap_create_info{};
		sampler_heap_create_info.usage       = vk::BufferUsageFlagBits::eDescriptorHeapEXT | vk::BufferUsageFlagBits::eShaderDeviceAddressKHR;
		sampler_heap_create_info.sharingMode = vk::SharingMode::eExclusive;
		sampler_heap_create_info.size        = sampler_heap_size;
		sampler_heap_create_info.setQueueFamilyIndices(p_logical_device.getQueueFamilyIndices().graphics);

		VmaAllocationCreateInfo heap_allocation_create_info{};
		heap_allocation_create_info.usage = VMA_MEMORY_USAGE_AUTO;
		heap_allocation_create_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

		vmaCreateBuffer(m_allocator->getAllocator(), reinterpret_cast<VkBufferCreateInfo *>(&sampler_heap_create_info), &heap_allocation_create_info,
						reinterpret_cast<VkBuffer *>(&m_heapBuffer), &m_heapAllocation, nullptr);

		vmaMapMemory(m_allocator->getAllocator(), m_heapAllocation, &m_mappedHeapMemory);
	}

	SamplerDescriptorHeap::~SamplerDescriptorHeap()
	{
		vmaUnmapMemory(m_allocator->getAllocator(), m_heapAllocation);
		vmaDestroyBuffer(m_allocator->getAllocator(), m_heapBuffer, m_heapAllocation);
	}

	auto SamplerDescriptorHeap::setSampler(uint32 p_slot, const vk::SamplerCreateInfo &p_sampler_create_info) -> void
	{
		m_samplerCreateInfos.push_back(p_sampler_create_info);

		vk::HostAddressRangeEXT host_range{m_hostAddressRanges.emplace_back()};
		host_range.address = reinterpret_cast<void *>(reinterpret_cast<uint64>(m_mappedHeapMemory) + static_cast<uint64>(p_slot) * m_samplerDescriptorSize);
		host_range.size    = m_samplerDescriptorSize;
	}

	auto SamplerDescriptorHeap::writeDescriptors() -> void
	{
		m_logicalDevice->getDevice().writeSamplerDescriptorsEXT(m_samplerCreateInfos, m_hostAddressRanges);
		m_samplerCreateInfos.clear();
		m_hostAddressRanges.clear();
	}
}
