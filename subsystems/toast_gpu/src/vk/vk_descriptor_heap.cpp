#include "toast_gpu/vk/vk_descriptor_heap.hpp"

#include "toast_gpu/vk/vk_command_buffer.hpp"
#include "toast_gpu/vk/vk_logical_device.hpp"

namespace toaster::gpu
{
	VKDescriptorHeap::VKDescriptorHeap(VKLogicalDevice *p_device) : m_device(p_device)
	{
		// get the heap properties. Vulkan raii does not yet have a function for ts... :(
		vk::PhysicalDeviceProperties2 device_props{};
		device_props.pNext = &m_heapProperties;
		m_device->getPhysicalDevice()->getVulkanPhysicalDevice().getProperties2(&device_props);

		vk::DeviceSize resource_heap_size{
			ALIGN(12 * m_heapProperties.bufferDescriptorSize + m_heapProperties.minResourceHeapReservedRange, m_heapProperties.resourceHeapAlignment)
		};

		BufferSpecInfo resource_heap_spec_info{};
		resource_heap_spec_info.usageFlags = vk::BufferUsageFlagBits2::eDescriptorHeapEXT | vk::BufferUsageFlagBits2::eShaderDeviceAddressKHR;
		m_resourceHeap                     = toaster::make_unique<Buffer>(m_device, resource_heap_size, resource_heap_spec_info);
	}

	auto VKDescriptorHeap::getHeapProperties() const -> const HeapProperties &
	{
		return m_heapProperties;
	}

	auto VKDescriptorHeap::getResourceHeap() const -> const VKBuffer &
	{
		return *m_resourceHeap;
	}

	auto VKDescriptorHeap::getNumBuffers() const -> uint32
	{
		return m_numBuffers;
	}

	auto VKDescriptorHeap::getNumImages() const -> uint32
	{
		return m_numImages;
	}

	auto VKDescriptorHeap::allocBuffer(const VKBuffer &p_buffer) -> void
	{
		void *mapped_resource_heap_memory{m_resourceHeap->mapMemory(m_resourceHeap->getSize())};

		vk::DeviceAddressRangeEXT buffer_range{p_buffer.getDeviceAddressRange()};

		vk::DeviceSize          uniform_descriptor_size{ALIGN(m_heapProperties.bufferDescriptorSize, m_heapProperties.bufferDescriptorAlignment)};
		vk::HostAddressRangeEXT host_range{};

		m_resourceHeapSize += m_heapProperties.bufferDescriptorSize;
		host_range.address = static_cast<uint8 *>(mapped_resource_heap_memory) + m_resourceHeapSize;
		host_range.size    = uniform_descriptor_size;

		vk::ResourceDescriptorInfoEXT resource_info{};
		resource_info.type               = vk::DescriptorType::eUniformBuffer;
		resource_info.data.pAddressRange = &buffer_range;

		m_device->getVulkanLogicalDevice().writeResourceDescriptorsEXT(resource_info, host_range);

		m_resourceHeap->unmapMemory();

		++m_numBuffers;
	}

	auto VKDescriptorHeap::bind(VKCommandBuffer *p_command_buffer) const -> void
	{
		TST_GPU_GET_VALID_CMD_BUFFER();
		vk::BindHeapInfoEXT resource_heap_bind_info{};
		resource_heap_bind_info.heapRange           = m_resourceHeap->getDeviceAddressRange();
		resource_heap_bind_info.reservedRangeOffset = m_resourceHeap->getSize() - m_heapProperties.minResourceHeapReservedRange;
		resource_heap_bind_info.reservedRangeSize   = m_heapProperties.minResourceHeapReservedRange;

		cmd->getVulkanCommandBuffer().bindResourceHeapEXT(resource_heap_bind_info);
	}
}
