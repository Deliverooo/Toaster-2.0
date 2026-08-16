#include "toast_gpu/material.hpp"

namespace toaster::gpu
{
	Material::Material(LogicalDevice &                      p_device, Allocator &p_allocator, ResourceDescriptorHeap &p_resource_heap,
					   const RefPtr<MaterialStructMapping> &p_struct_mapping) : m_allocator(&p_allocator), m_resourceDescriptorHeap(&p_resource_heap),
																				m_structMapping(p_struct_mapping)
	{
		m_materialBuffers.resize(3u);
		m_materialBufferAllocations.resize(3u);
		m_mappedMaterialBuffers.resize(3u);
		m_materialHeapIDs.resize(3u);

		vk::BufferCreateInfo material_buffer_create_info{};
		material_buffer_create_info.usage       = vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddressKHR;
		material_buffer_create_info.sharingMode = vk::SharingMode::eExclusive;
		material_buffer_create_info.size        = m_structMapping->size;
		material_buffer_create_info.setQueueFamilyIndices(p_device.getQueueFamilyIndices().graphics);

		VmaAllocationCreateInfo material_buffer_allocation_create_info{};
		material_buffer_allocation_create_info.usage = VMA_MEMORY_USAGE_AUTO;
		material_buffer_allocation_create_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

		for (uint32 i{0u}; i < 3u; ++i)
		{
			vmaCreateBuffer(m_allocator->getAllocator(), reinterpret_cast<VkBufferCreateInfo *>(&material_buffer_create_info), &material_buffer_allocation_create_info,
							reinterpret_cast<VkBuffer *>(&m_materialBuffers[i]), &m_materialBufferAllocations[i], nullptr);

			vmaMapMemory(m_allocator->getAllocator(), m_materialBufferAllocations[i], &m_mappedMaterialBuffers[i]);

			m_materialHeapIDs[i] = m_resourceDescriptorHeap->allocBufferSlot();

			vk::DeviceAddress         buffer_address{p_device.getDevice().getBufferAddress({m_materialBuffers[i]})};
			vk::DeviceAddressRangeKHR buffer_address_range{buffer_address, m_structMapping->size};
			m_resourceDescriptorHeap->setBuffer(m_materialHeapIDs[i], buffer_address_range, vk::DescriptorType::eUniformBuffer);
		}

		m_materialData = new uint8[m_structMapping->size];
	}

	Material::~Material()
	{
		for (uint32 i{0u}; i < 3u; ++i)
		{
			vmaUnmapMemory(m_allocator->getAllocator(), m_materialBufferAllocations[i]);
			vmaDestroyBuffer(m_allocator->getAllocator(), m_materialBuffers[i], m_materialBufferAllocations[i]);
			m_resourceDescriptorHeap->freeBufferSlot(m_materialHeapIDs[i]);
		}

		delete[] m_materialData;
	}

	auto Material::update(uint32 p_frame_index) -> void
	{
		if (m_numFramesDirty > 0)
		{
			std::memcpy(m_mappedMaterialBuffers[p_frame_index], m_materialData, m_structMapping->size);
			--m_numFramesDirty;
		}
	}

	auto Material::_set(const String &p_name, const void *p_value) -> void
	{
		m_numFramesDirty = 3u;

		const auto it{m_structMapping->members.find(p_name)};
		if (it == m_structMapping->members.end())
		{
			TST_PERMA_ASSERT_MSG(false, "material does not have specified struct member");
			return;
		}

		const uint64 offset{it->second.offset};
		const uint64 size{it->second.size};
		std::memcpy(reinterpret_cast<void *>(reinterpret_cast<uint64>(m_materialData) + offset), p_value, size);
	}
}
