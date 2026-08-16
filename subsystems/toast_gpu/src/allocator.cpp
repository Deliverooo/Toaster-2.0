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
}
