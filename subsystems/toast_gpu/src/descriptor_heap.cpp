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

		vk::DeviceAddressRangeKHR device_address_range{};
		device_address_range.address   = p_logical_device.getDevice().getBufferAddress({m_heapBuffer});
		device_address_range.size      = resource_heap_size;
		m_bindInfo.heapRange           = device_address_range;
		m_bindInfo.reservedRangeOffset = resource_heap_size - heap_props.minResourceHeapReservedRange;
		m_bindInfo.reservedRangeSize   = heap_props.minResourceHeapReservedRange;
	}

	ResourceDescriptorHeap::~ResourceDescriptorHeap()
	{
		vmaUnmapMemory(m_allocator->getAllocator(), m_heapAllocation);
		vmaDestroyBuffer(m_allocator->getAllocator(), m_heapBuffer, m_heapAllocation);
	}

	auto ResourceDescriptorHeap::setBuffer(DescriptorSlot p_slot, const vk::DeviceAddressRangeKHR &p_address_range, vk::DescriptorType p_descriptor_type) -> void
	{
		m_bufferDeviceAddressRanges.push_back(p_address_range);

		vk::HostAddressRangeEXT &host_range{m_bufferHostAddressRanges.emplace_back()};
		host_range.address = reinterpret_cast<void *>(reinterpret_cast<uint64>(m_mappedHeapMemory) + static_cast<uint64>(p_slot) * m_bufferDescriptorSize);
		host_range.size    = m_bufferDescriptorSize;

		vk::ResourceDescriptorInfoEXT &resource_info{m_bufferResourceInfos.emplace_back()};
		resource_info.type = p_descriptor_type;
	}

	auto ResourceDescriptorHeap::setImage(DescriptorSlot     p_slot, const vk::ImageViewCreateInfo &p_image_view_create_info, vk::ImageLayout p_image_layout,
										  vk::DescriptorType p_descriptor_type) -> void
	{
		m_imageViewCreateInfos.push_back(p_image_view_create_info);

		vk::HostAddressRangeEXT &host_range{m_imageHostAddressRanges.emplace_back()};
		host_range.address = reinterpret_cast<void *>(reinterpret_cast<uint64>(m_mappedHeapMemory) + m_imageSegmentOffset + static_cast<uint64>(p_slot) *
													  m_imageDescriptorSize);
		host_range.size = m_imageDescriptorSize;

		vk::ImageDescriptorInfoEXT image_info{m_imageDescriptorInfos.emplace_back()};
		image_info.layout = p_image_layout;

		vk::ResourceDescriptorInfoEXT &resource_info{m_imageResourceInfos.emplace_back()};
		resource_info.type = p_descriptor_type;
	}

	auto ResourceDescriptorHeap::writeDescriptors() -> void
	{
		for (uint32 i{0u}; i < m_bufferResourceInfos.size(); ++i)
			m_bufferResourceInfos[i].data.pAddressRange = &m_bufferDeviceAddressRanges[i];

		for (uint32 i{0u}; i < m_imageResourceInfos.size(); ++i)
		{
			m_imageDescriptorInfos[i].pView     = &m_imageViewCreateInfos[i];
			m_imageResourceInfos[i].data.pImage = &m_imageDescriptorInfos[i];
		}

		if (!m_bufferResourceInfos.empty())
		{
			m_logicalDevice->getDevice().writeResourceDescriptorsEXT(m_bufferResourceInfos, m_bufferHostAddressRanges);
			m_bufferResourceInfos.clear();
			m_bufferHostAddressRanges.clear();
			m_bufferDeviceAddressRanges.clear();
		}

		if (!m_imageResourceInfos.empty())
		{
			m_logicalDevice->getDevice().writeResourceDescriptorsEXT(m_imageResourceInfos, m_imageHostAddressRanges);
			m_imageHostAddressRanges.clear();
			m_imageViewCreateInfos.clear();
			m_imageDescriptorInfos.clear();
			m_imageResourceInfos.clear();
		}
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

		vk::DeviceAddressRangeKHR device_address_range{};
		device_address_range.address   = p_logical_device.getDevice().getBufferAddress({m_heapBuffer});
		device_address_range.size      = sampler_heap_size;
		m_bindInfo.heapRange           = device_address_range;
		m_bindInfo.reservedRangeOffset = sampler_heap_size - heap_props.minSamplerHeapReservedRange;
		m_bindInfo.reservedRangeSize   = heap_props.minSamplerHeapReservedRange;
	}

	SamplerDescriptorHeap::~SamplerDescriptorHeap()
	{
		vmaUnmapMemory(m_allocator->getAllocator(), m_heapAllocation);
		vmaDestroyBuffer(m_allocator->getAllocator(), m_heapBuffer, m_heapAllocation);
	}

	auto SamplerDescriptorHeap::setSampler(DescriptorSlot p_slot, const vk::SamplerCreateInfo &p_sampler_create_info) -> void
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
