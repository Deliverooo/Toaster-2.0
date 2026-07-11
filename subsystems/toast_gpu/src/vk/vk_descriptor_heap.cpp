#include "toast_gpu/vk/vk_descriptor_heap.hpp"

#include "toast_gpu/vk/vk_command_buffer.hpp"
#include "toast_gpu/vk/vk_gpu_context.hpp"
#include "toast_gpu/vk/vk_logical_device.hpp"

namespace toaster::gpu
{
	DescriptorSlotManager::DescriptorSlotManager(uint32 p_capacity)
	{
		m_freeSlots.reserve(p_capacity);
		for (int32 i{static_cast<int32>(p_capacity) - 1}; i >= 0; --i)
			m_freeSlots.push_back(static_cast<uint32>(i));
	}

	auto DescriptorSlotManager::allocSlot() -> DescriptorSlot
	{
		if (m_freeSlots.empty())
		{
			TST_PERMA_ASSERT(false);
		}
		DescriptorSlot allocated{m_freeSlots.back()};
		m_freeSlots.pop_back();
		return allocated;
	}

	auto DescriptorSlotManager::freeSlot(DescriptorSlot p_slot) -> void
	{
		m_freeSlots.push_back(p_slot);
	}

	VKDescriptorHeap::VKDescriptorHeap(VKGPUContext &p_gpu_ctx) : m_gpuCtx(&p_gpu_ctx)
	{
		// get the heap properties. Vulkan raii does not yet have a function for ts... :(
		vk::PhysicalDeviceProperties2 device_props{};
		device_props.pNext = &m_heapProperties;
		m_gpuCtx->getPhysicalDevice()->getVulkanPhysicalDevice().getProperties2(&device_props);

		// create the resource heap
		{
			m_bufferDescriptorSize = ALIGN(m_heapProperties.bufferDescriptorSize, m_heapProperties.bufferDescriptorAlignment);
			m_imageDescriptorSize  = ALIGN(m_heapProperties.imageDescriptorSize, m_heapProperties.imageDescriptorAlignment);

			LOG_WARN("Buffer Descriptor size: {}", m_bufferDescriptorSize);
			LOG_WARN("Image Descriptor size: {}", m_imageDescriptorSize);

			m_bufferArrayOffset = 0u;
			m_bufferArraySize   = (maxUBOs * m_bufferDescriptorSize);

			m_imageArrayOffset = ALIGN(m_bufferArraySize, m_heapProperties.imageDescriptorAlignment);
			m_imageArraySize   = (maxImages * m_imageDescriptorSize);

			vk::DeviceSize resource_heap_size{
				ALIGN(m_bufferArraySize + m_imageArraySize + m_heapProperties.minResourceHeapReservedRange, m_heapProperties.resourceHeapAlignment)
			};

			m_bufferSlotManager = DescriptorSlotManager{static_cast<uint32>(maxUBOs)};
			m_imageSlotManager  = DescriptorSlotManager{static_cast<uint32>(maxImages)};

			BufferSpecInfo resource_heap_spec_info{};
			resource_heap_spec_info.usageFlags = vk::BufferUsageFlagBits2::eDescriptorHeapEXT | vk::BufferUsageFlagBits2::eShaderDeviceAddressKHR;
			m_resourceHeap                     = toaster::make_unique<Buffer>(*m_gpuCtx, resource_heap_size, resource_heap_spec_info);

			m_resourceHeapMemory = m_resourceHeap->mapMemory(resource_heap_size);
		}

		// create the sampler heap
		{
			vk::DeviceSize sampler_heap_size{
				ALIGN((maxSamplers * m_heapProperties.samplerDescriptorSize) + m_heapProperties.minSamplerHeapReservedRange, m_heapProperties.samplerHeapAlignment)
			};

			m_samplerSlotManager = DescriptorSlotManager{static_cast<uint32>(maxSamplers * m_heapProperties.samplerDescriptorSize)};

			BufferSpecInfo sampler_heap_spec_info{};
			sampler_heap_spec_info.usageFlags = vk::BufferUsageFlagBits2::eDescriptorHeapEXT | vk::BufferUsageFlagBits2::eShaderDeviceAddressKHR;
			m_samplerHeap                     = toaster::make_unique<Buffer>(*m_gpuCtx, sampler_heap_size, sampler_heap_spec_info);

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

	auto VKDescriptorHeap::getResourceHeapMemory() const -> void *
	{
		return m_resourceHeapMemory;
	}

	auto VKDescriptorHeap::getSamplerHeapMemory() const -> void *
	{
		return m_samplerHeapMemory;
	}

	auto VKDescriptorHeap::getBufferDescriptorSize() const -> vk::DeviceSize
	{
		return m_bufferDescriptorSize;
	}

	auto VKDescriptorHeap::getImageDescriptorSize() const -> vk::DeviceSize
	{
		return m_imageDescriptorSize;
	}

	auto VKDescriptorHeap::getBufferOffset() const -> uintptr_t
	{
		return m_bufferArrayOffset;
	}

	auto VKDescriptorHeap::getImageOffset() const -> uintptr_t
	{
		return m_imageArrayOffset;
	}

	auto VKDescriptorHeap::allocBuffer(const Buffer &p_buffer, bool p_storage) -> DescriptorSlot
	{
		DescriptorSlot allocated_slot{m_bufferSlotManager.allocSlot()};
		setBuffer(allocated_slot, p_buffer, p_storage);
		return allocated_slot;
	}

	auto VKDescriptorHeap::allocImage(const VKRawImage &p_image, bool p_storage, uint32 p_mip_level) -> DescriptorSlot
	{
		DescriptorSlot allocated_slot{m_imageSlotManager.allocSlot()};
		setImage(allocated_slot, p_image, p_storage, p_mip_level);
		return allocated_slot;
	}

	auto VKDescriptorHeap::allocSampler(const vk::SamplerCreateInfo &p_sampler) -> DescriptorSlot
	{
		DescriptorSlot allocated_slot{m_samplerSlotManager.allocSlot()};
		setSampler(allocated_slot, p_sampler);
		return allocated_slot;
	}

	auto VKDescriptorHeap::setBuffer(DescriptorSlot p_slot, const Buffer &p_buffer, bool p_storage) -> void
	{
		vk::DeviceAddressRangeEXT buffer_range{p_buffer.getDeviceAddressRange()};

		vk::HostAddressRangeEXT host_range{};
		host_range.address = static_cast<uint8 *>(m_resourceHeapMemory) + m_bufferArrayOffset + p_slot * m_bufferDescriptorSize;
		host_range.size    = m_bufferDescriptorSize;

		vk::ResourceDescriptorInfoEXT resource_info{};
		resource_info.type               = p_storage ? vk::DescriptorType::eStorageBuffer : vk::DescriptorType::eUniformBuffer;
		resource_info.data.pAddressRange = &buffer_range;

		// LOG_INFO("Buffer | Slot: {} | Heap ID: {}", p_slot, p_slot + (m_bufferArrayOffset / m_bufferDescriptorSize));

		m_gpuCtx->getLogicalDevice()->getVulkanLogicalDevice().writeResourceDescriptorsEXT(resource_info, host_range);
	}

	auto VKDescriptorHeap::setImage(DescriptorSlot p_slot, const RawImage &p_image, bool p_storage, uint32 p_mip_level) -> void
	{
		vk::HostAddressRangeEXT host_range{};
		host_range.address = static_cast<uint8 *>(m_resourceHeapMemory) + m_imageArrayOffset + p_slot * m_imageDescriptorSize;
		host_range.size    = m_imageDescriptorSize;

		vk::ImageDescriptorInfoEXT image_info{};
		vk::ImageViewCreateInfo    image_view_create_info{};
		image_info.layout = (p_storage) ? vk::ImageLayout::eGeneral : vk::ImageLayout::eShaderReadOnlyOptimal;
		if (p_mip_level == UINT32_MAX)
			image_view_create_info = p_image.getImageViewCreateInfo();
		else
			image_view_create_info = p_image.getMipImageViewCreateInfo(p_mip_level, 1u);
		image_info.pView = &image_view_create_info;

		vk::ResourceDescriptorInfoEXT resource_info{};
		resource_info.type        = (p_storage) ? vk::DescriptorType::eStorageImage : vk::DescriptorType::eSampledImage;
		resource_info.data.pImage = &image_info;

		// LOG_INFO("Image | Slot: {} | Heap ID: {}", p_slot, p_slot + (m_imageArrayOffset / m_imageDescriptorSize));

		m_gpuCtx->getLogicalDevice()->getVulkanLogicalDevice().writeResourceDescriptorsEXT(resource_info, host_range);
	}

	auto VKDescriptorHeap::setSampler(DescriptorSlot p_slot, const vk::SamplerCreateInfo &p_sampler) -> void
	{
		vk::DeviceSize          sampler_descriptor_size{ALIGN(m_heapProperties.samplerDescriptorSize, m_heapProperties.samplerDescriptorAlignment)};
		vk::HostAddressRangeEXT host_range{};

		host_range.address = static_cast<uint8 *>(m_samplerHeapMemory) + (p_slot * sampler_descriptor_size);
		host_range.size    = sampler_descriptor_size;

		m_gpuCtx->getLogicalDevice()->getVulkanLogicalDevice().writeSamplerDescriptorsEXT(p_sampler, host_range);
	}

	auto VKDescriptorHeap::freeBuffer(DescriptorSlot p_slot) -> void
	{
		m_bufferSlotManager.freeSlot(p_slot);
	}

	auto VKDescriptorHeap::freeImage(DescriptorSlot p_slot) -> void
	{
		m_imageSlotManager.freeSlot(p_slot);
	}

	auto VKDescriptorHeap::freeSampler(DescriptorSlot p_slot) -> void
	{
		m_samplerSlotManager.freeSlot(p_slot);
	}
}
