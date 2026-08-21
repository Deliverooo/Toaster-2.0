#include "toast_gpu/allocator.hpp"

namespace toaster::gpu
{
	Allocator::Allocator(Instance &p_instance, PhysicalDevice &p_physical_device, LogicalDevice &p_logical_device)
	{
		VmaAllocatorCreateInfo allocator_create_info{};
		allocator_create_info.instance       = p_instance.getInstance();
		allocator_create_info.physicalDevice = p_physical_device.getPhysicalDevice();
		allocator_create_info.device         = p_logical_device.getDevice();
		allocator_create_info.flags          = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

		vmaCreateAllocator(&allocator_create_info, &m_allocator);
	}

	Allocator::~Allocator()
	{
		vmaDestroyAllocator(m_allocator);
	}

	auto Allocator::createBuffer(uint64 p_size, vk::BufferUsageFlags p_usage_flags, VmaAllocationCreateFlags p_allocation_flags) -> GPUBuffer
	{
		vk::BufferCreateInfo buffer_create_info{};
		buffer_create_info.size        = p_size;
		buffer_create_info.usage       = p_usage_flags;
		buffer_create_info.sharingMode = vk::SharingMode::eExclusive;

		VmaAllocationCreateInfo buffer_allocation_create_info{};
		buffer_allocation_create_info.usage = VMA_MEMORY_USAGE_AUTO;
		buffer_allocation_create_info.flags = p_allocation_flags;

		GPUBuffer out_buffer{};
		vmaCreateBuffer(m_allocator, reinterpret_cast<VkBufferCreateInfo *>(&buffer_create_info), &buffer_allocation_create_info,
						reinterpret_cast<VkBuffer *>(&out_buffer.buffer), &out_buffer.allocation, nullptr);
		return out_buffer;
	}

	auto Allocator::createBuffer(uint64         p_size, vk::BufferUsageFlags p_usage_flags, VmaAllocationCreateFlags p_allocation_flags, vk::Buffer &p_out_buffer,
								 VmaAllocation &p_out_allocation, void **    p_out_mapped) -> void
	{
		vk::BufferCreateInfo buffer_create_info{};
		buffer_create_info.size        = p_size;
		buffer_create_info.usage       = p_usage_flags;
		buffer_create_info.sharingMode = vk::SharingMode::eExclusive;

		VmaAllocationCreateInfo buffer_allocation_create_info{};
		buffer_allocation_create_info.usage = VMA_MEMORY_USAGE_AUTO;
		buffer_allocation_create_info.flags = p_allocation_flags;

		VmaAllocationInfo allocation_info{};
		vmaCreateBuffer(m_allocator, reinterpret_cast<VkBufferCreateInfo *>(&buffer_create_info), &buffer_allocation_create_info,
						reinterpret_cast<VkBuffer *>(&p_out_buffer), &p_out_allocation, &allocation_info);

		if (p_out_mapped)
			*p_out_mapped = allocation_info.pMappedData;
	}

	auto Allocator::destroyBuffer(GPUBuffer &p_buffer) const -> void
	{
		vmaDestroyBuffer(m_allocator, p_buffer.buffer, p_buffer.allocation);
		p_buffer.buffer     = nullptr;
		p_buffer.allocation = nullptr;
	}

	auto Allocator::destroyBuffer(vk::Buffer &p_buffer, VmaAllocation &p_allocation) const -> void
	{
		vmaDestroyBuffer(m_allocator, p_buffer, p_allocation);
		p_buffer     = nullptr;
		p_allocation = nullptr;
	}

	auto Allocator::createImage(const vk::ImageCreateInfo &p_create_info, VmaAllocationCreateFlags p_allocation_flags) -> GPUImage
	{
		VmaAllocationCreateInfo image_allocation_create_info{};
		image_allocation_create_info.usage = VMA_MEMORY_USAGE_AUTO;
		image_allocation_create_info.flags = p_allocation_flags;

		GPUImage out_image{};
		vmaCreateImage(m_allocator, reinterpret_cast<const VkImageCreateInfo *>(&p_create_info), &image_allocation_create_info,
					   reinterpret_cast<VkImage *>(&out_image.image), &out_image.allocation, nullptr);
		return out_image;
	}

	auto Allocator::createImage(const vk::ImageCreateInfo &p_create_info, VmaAllocationCreateFlags p_allocation_flags, vk::Image &p_out_image,
								VmaAllocation &            p_out_allocation) -> void
	{
		VmaAllocationCreateInfo image_allocation_create_info{};
		image_allocation_create_info.usage = VMA_MEMORY_USAGE_AUTO;
		image_allocation_create_info.flags = p_allocation_flags;

		vmaCreateImage(m_allocator, reinterpret_cast<const VkImageCreateInfo *>(&p_create_info), &image_allocation_create_info, reinterpret_cast<VkImage *>(&p_out_image),
					   &p_out_allocation, nullptr);
	}

	auto Allocator::destroyImage(GPUImage &p_image) const -> void
	{
		vmaDestroyImage(m_allocator, p_image.image, p_image.allocation);
		p_image.image      = nullptr;
		p_image.allocation = nullptr;
	}

	auto Allocator::destroyImage(vk::Image &p_image, VmaAllocation &p_allocation) const -> void
	{
		vmaDestroyImage(m_allocator, p_image, p_allocation);
		p_image      = nullptr;
		p_allocation = nullptr;
	}
}
