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

		// create the resource heap
		{
			vk::DeviceSize buffer_descriptor_size{ALIGN(m_heapProperties.bufferDescriptorSize, m_heapProperties.bufferDescriptorAlignment)};
			vk::DeviceSize image_descriptor_size{ALIGN(m_heapProperties.imageDescriptorSize, m_heapProperties.imageDescriptorAlignment)};

			m_bufferArrayOffset = 0u;
			m_bufferArraySize   = (maxUBOs * buffer_descriptor_size);

			m_imageArrayOffset = m_bufferArraySize;
			m_imageArraySize   = (maxImages * image_descriptor_size);

			vk::DeviceSize resource_heap_size{
				ALIGN(m_bufferArraySize + m_imageArraySize + m_heapProperties.minResourceHeapReservedRange, m_heapProperties.resourceHeapAlignment)
			};

			BufferSpecInfo resource_heap_spec_info{};
			resource_heap_spec_info.usageFlags = vk::BufferUsageFlagBits2::eDescriptorHeapEXT | vk::BufferUsageFlagBits2::eShaderDeviceAddressKHR;
			m_resourceHeap                     = toaster::make_unique<Buffer>(m_device, resource_heap_size, resource_heap_spec_info);

			m_resourceHeapMemory = m_resourceHeap->mapMemory(resource_heap_size);
		}

		// create the sampler heap
		{
			vk::DeviceSize sampler_heap_size{
				ALIGN((maxSamplers * m_heapProperties.samplerDescriptorSize) + m_heapProperties.minSamplerHeapReservedRange, m_heapProperties.samplerHeapAlignment)
			};

			BufferSpecInfo sampler_heap_spec_info{};
			sampler_heap_spec_info.usageFlags = vk::BufferUsageFlagBits2::eDescriptorHeapEXT | vk::BufferUsageFlagBits2::eShaderDeviceAddressKHR;
			m_samplerHeap                     = toaster::make_unique<Buffer>(m_device, sampler_heap_size, sampler_heap_spec_info);

			m_samplerHeapMemory = m_samplerHeap->mapMemory(sampler_heap_size);
		}
	}

	VKDescriptorHeap::~VKDescriptorHeap()
	{
		m_resourceHeap->unmapMemory();
		m_samplerHeap->unmapMemory();
	}

	auto VKDescriptorHeap::getHeapProperties() const -> const HeapProperties &
	{
		return m_heapProperties;
	}

	auto VKDescriptorHeap::getResourceHeap() const -> const VKBuffer &
	{
		return *m_resourceHeap;
	}

	auto VKDescriptorHeap::getSamplerHeap() const -> const VKBuffer &
	{
		return *m_samplerHeap;
	}

	auto VKDescriptorHeap::getBaseBufferAddress() const -> vk::DeviceAddress
	{
		return m_resourceHeap->getDeviceAddress() + m_bufferArrayOffset;
	}

	auto VKDescriptorHeap::getBaseImageAddress() const -> vk::DeviceAddress
	{
		return m_resourceHeap->getDeviceAddress() + m_imageArrayOffset;
	}

	auto VKDescriptorHeap::getSamplerIndex() const -> uint32
	{
		return m_samplerIndex;
	}

	auto VKDescriptorHeap::allocBuffer(const VKBuffer &p_buffer) -> DescriptorSlot
	{
		vk::DeviceAddressRangeEXT buffer_range{p_buffer.getDeviceAddressRange()};

		vk::DeviceSize uniform_descriptor_size{ALIGN(m_heapProperties.bufferDescriptorSize, m_heapProperties.bufferDescriptorAlignment)};

		uintptr_t               byte_offset{m_bufferArrayOffset + static_cast<uintptr_t>(m_heapResourceIndex) * uniform_descriptor_size};
		vk::HostAddressRangeEXT host_range{};
		host_range.address = (void *) (reinterpret_cast<uintptr_t>(m_resourceHeapMemory) + byte_offset);
		host_range.size    = uniform_descriptor_size;

		vk::ResourceDescriptorInfoEXT resource_info{};
		resource_info.type               = vk::DescriptorType::eUniformBuffer;
		resource_info.data.pAddressRange = &buffer_range;

		m_device->getVulkanLogicalDevice().writeResourceDescriptorsEXT(resource_info, host_range);

		return m_heapResourceIndex++;
	}

	auto VKDescriptorHeap::allocImage(const VKRawImage &p_image) -> DescriptorSlot
	{
		vk::DeviceSize image_descriptor_size{ALIGN(m_heapProperties.imageDescriptorSize, m_heapProperties.imageDescriptorAlignment)};

		uintptr_t               byte_offset{ m_heapResourceIndex * image_descriptor_size};
		vk::HostAddressRangeEXT host_range{};
		host_range.address = static_cast<uint8 *>(m_resourceHeapMemory) + byte_offset;
		host_range.size    = image_descriptor_size;

		vk::ImageDescriptorInfoEXT image_info{};
		image_info.layout = p_image.getCurrentImageLayout();
		image_info.pView  = &p_image.getImageViewCreateInfo();

		vk::ResourceDescriptorInfoEXT resource_info{};
		resource_info.type        = vk::DescriptorType::eSampledImage;
		resource_info.data.pImage = &image_info;

		m_device->getVulkanLogicalDevice().writeResourceDescriptorsEXT(resource_info, host_range);

		return m_heapResourceIndex++;
	}

	auto VKDescriptorHeap::allocSampler(const vk::SamplerCreateInfo &p_sampler) -> DescriptorSlot
	{
		vk::DeviceSize          sampler_descriptor_size{ALIGN(m_heapProperties.samplerDescriptorSize, m_heapProperties.samplerDescriptorAlignment)};
		vk::HostAddressRangeEXT host_range{};

		host_range.address = static_cast<uint8 *>(m_samplerHeapMemory) + (m_samplerIndex * sampler_descriptor_size);
		host_range.size    = sampler_descriptor_size;

		m_device->getVulkanLogicalDevice().writeSamplerDescriptorsEXT(p_sampler, host_range);

		return m_samplerIndex++;
	}

	auto VKDescriptorHeap::bind(VKCommandBuffer *p_command_buffer) const -> void
	{
		TST_GPU_GET_VALID_CMD_BUFFER();

		// bind the resource heap
		{
			vk::BindHeapInfoEXT resource_heap_bind_info{};
			resource_heap_bind_info.heapRange           = m_resourceHeap->getDeviceAddressRange();
			resource_heap_bind_info.reservedRangeOffset = m_resourceHeap->getSize() - m_heapProperties.minResourceHeapReservedRange;
			resource_heap_bind_info.reservedRangeSize   = m_heapProperties.minResourceHeapReservedRange;

			cmd->getVulkanCommandBuffer().bindResourceHeapEXT(resource_heap_bind_info);
		}

		// bind the sampler heap
		{
			vk::BindHeapInfoEXT sampler_heap_bind_info{};
			sampler_heap_bind_info.heapRange           = m_samplerHeap->getDeviceAddressRange();
			sampler_heap_bind_info.reservedRangeOffset = m_samplerHeap->getSize() - m_heapProperties.minSamplerHeapReservedRange;
			sampler_heap_bind_info.reservedRangeSize   = m_heapProperties.minSamplerHeapReservedRange;

			cmd->getVulkanCommandBuffer().bindSamplerHeapEXT(sampler_heap_bind_info);
		}
	}
}
