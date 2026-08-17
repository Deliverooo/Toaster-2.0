#include "toast_gpu/material.hpp"

namespace toaster::gpu
{
	MaterialManager::MaterialManager(const LogicalDevice &p_device, Allocator &p_allocator, const RefPtr<MaterialStructMapping> &p_struct_mapping,
									 uint32               p_max_materials) : m_allocator(&p_allocator), m_structMapping(p_struct_mapping), m_maxMaterials(p_max_materials)
	{
		m_pool.reserve(p_max_materials);

		m_materialBuffers.resize(3u);
		m_materialBufferAllocations.resize(3u);
		m_mappedMaterialBuffers.resize(3u);
		m_materialBufferDeviceAddresses.resize(3u);

		vk::BufferCreateInfo material_buffer_create_info{};
		material_buffer_create_info.size        = m_maxMaterials * m_structMapping->size;
		material_buffer_create_info.usage       = vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress;
		material_buffer_create_info.sharingMode = vk::SharingMode::eExclusive;

		VmaAllocationCreateInfo material_buffer_allocation_create_info{};
		material_buffer_allocation_create_info.usage = VMA_MEMORY_USAGE_AUTO;
		material_buffer_allocation_create_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

		for (uint32 i{0u}; i < 3u; ++i)
		{
			vmaCreateBuffer(m_allocator->getAllocator(), reinterpret_cast<VkBufferCreateInfo *>(&material_buffer_create_info), &material_buffer_allocation_create_info,
							reinterpret_cast<VkBuffer *>(&m_materialBuffers[i]), &m_materialBufferAllocations[i], nullptr);

			vmaMapMemory(m_allocator->getAllocator(), m_materialBufferAllocations[i], reinterpret_cast<void **>(&m_mappedMaterialBuffers[i]));

			m_materialBufferDeviceAddresses[i] = p_device.getDevice().getBufferAddress({m_materialBuffers[i]});
		}
	}

	MaterialManager::~MaterialManager()
	{
		for (uint32 i{0u}; i < 3u; ++i)
		{
			vmaUnmapMemory(m_allocator->getAllocator(), m_materialBufferAllocations[i]);
			vmaDestroyBuffer(m_allocator->getAllocator(), m_materialBuffers[i], m_materialBufferAllocations[i]);
		}
	}

	auto MaterialManager::createMaterial() -> MaterialHandle
	{
		const MaterialHandle out_handle{m_pool.create({})};
		m_pool._data[out_handle.id].data = new uint8[m_structMapping->size];
		return out_handle;
	}

	auto MaterialManager::destroyMaterial(MaterialHandle p_handle) -> void
	{
		if (!isValid(p_handle))
			TST_PERMA_ASSERT(false);
		m_pool.destroy(p_handle);
		delete[] m_pool._data[p_handle.id].data;
	}

	auto MaterialManager::isValid(MaterialHandle p_handle) const -> bool
	{
		return m_pool.isValid(p_handle);
	}

	auto MaterialManager::getData(MaterialHandle p_handle) -> void *
	{
		return &m_pool.getData(p_handle)->data;
	}

	auto MaterialManager::markMaterialDirty(MaterialHandle p_handle) -> void
	{
		if (!isValid(p_handle))
			TST_PERMA_ASSERT(false);

		m_pool._data[p_handle.id].framesDirty = 3u;
	}

	auto MaterialManager::update(uint32 p_frame_index) -> void
	{
		for (uint32 i{0u}; i < m_pool.getSize(); ++i)
		{
			auto &entry{m_pool._data[i]};

			if (m_pool._alive[i] && entry.framesDirty > 0)
			{
				--entry.framesDirty;
				std::memcpy(&m_mappedMaterialBuffers[p_frame_index][i], entry.data, m_structMapping->size);
			}
		}
	}

	auto MaterialManager::_set(MaterialHandle p_handle, const String &p_name, const void *p_data) -> void
	{
		m_pool._data[p_handle.id].framesDirty = 3u;

		const auto it{m_structMapping->members.find(p_name)};
		if (it == m_structMapping->members.end())
		{
			TST_PERMA_ASSERT_MSG(false, "material does not have specified struct member");
			return;
		}

		const uint64 offset{it->second.offset};
		const uint64 size{it->second.size};
		std::memcpy(reinterpret_cast<void *>(reinterpret_cast<uint64>(m_pool._data[p_handle.id].data) + offset), p_data, size);
	}
}
