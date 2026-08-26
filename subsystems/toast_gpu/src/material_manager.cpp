#include "toast_gpu/material_manager.hpp"

namespace toaster::gpu
{
	MaterialManager::MaterialManager(Device &p_device, uint32 p_max_pool_size_bytes) : m_device(&p_device), m_maxPoolSize(p_max_pool_size_bytes)
	{
		VmaVirtualBlockCreateInfo block_create_info{};
		block_create_info.size = m_maxPoolSize;
		vmaCreateVirtualBlock(&block_create_info, &m_virtualBlock);

		BufferDesc material_buffer_desc{};
		material_buffer_desc.size             = m_maxPoolSize;
		material_buffer_desc.usageFlags       = vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer;
		material_buffer_desc.memoryProperties = EMemoryProperties::eHostVisibleCoherent;

		m_materialBuffers.resize(3u);

		for (auto &buffer: m_materialBuffers)
			buffer = m_device->createBuffer(material_buffer_desc);

		m_pool.setUserData(this);
		m_pool.setDestroyCallback(+[](void *p_user_data, MaterialHandle p_handle) -> void
		{
			auto ts{static_cast<MaterialManager *>(p_user_data)};

			MaterialData *material_data{&ts->m_pool._data[p_handle.id]};
			material_data->data.clear();         // We don't need to defer this
			material_data->textureRefs.fill({}); // Texture destruction is already queued

			ts->m_device->submitDeletion([data = *material_data, virtual_block = ts->m_virtualBlock]() mutable noexcept -> void // Copy
			{
				_destroyMaterialData(data, virtual_block);
			});
		});
	}

	MaterialManager::~MaterialManager()
	{
		for (auto &buffer: m_materialBuffers)
			m_device->destroyBuffer(buffer);

		// For safety...
		for (uint32 i{0u}; i < m_pool.getSize(); ++i)
		{
			if (m_pool._alive[i])
			{
				MaterialData *material_data{&m_pool._data[i]};
				material_data->data.clear();         // We don't need to defer this
				material_data->textureRefs.fill({}); // Texture destruction is already queued

				m_device->submitDeletion([data = *material_data, virtual_block = m_virtualBlock]() mutable noexcept -> void // Copy
				{
					_destroyMaterialData(data, virtual_block);
				});
			}
		}

		m_device->submitDeletion([virtual_block = m_virtualBlock] mutable noexcept -> void
		{
			vmaDestroyVirtualBlock(virtual_block);
		});
	}

	auto MaterialManager::createMaterial(uint32 p_size_bytes) -> MaterialHandle
	{
		MaterialData out_data{};
		out_data.data.resize(p_size_bytes, 0);

		VmaVirtualAllocationCreateInfo virtual_allocation_create_info{};
		virtual_allocation_create_info.size      = p_size_bytes;
		virtual_allocation_create_info.alignment = 16u;

		vk::Result res{vmaVirtualAllocate(m_virtualBlock, &virtual_allocation_create_info, &out_data.virtualAllocation, &out_data.allocationOffset)};
		TST_PERMA_ASSERT(res == vk::Result::eSuccess);

		out_data.allocationSize = p_size_bytes;
		out_data.framesDirty    = 3u;

		return m_pool.create(out_data);
	}

	auto MaterialManager::createSharedMaterial(uint32 p_size_bytes) -> SharedMaterial
	{
		return SharedMaterial{createMaterial(p_size_bytes), &m_pool};
	}

	auto MaterialManager::getMaterialDeviceAddress(MaterialHandle p_handle, uint32 p_frame_index) const -> vk::DeviceAddress
	{
		const MaterialData *data{getData(p_handle)};
		TST_PERMA_ASSERT(data);

		vk::DeviceAddress base_address{m_device->getBufferData(m_materialBuffers[p_frame_index])->address};
		return base_address + data->allocationOffset;
	}

	auto MaterialManager::setTextureRef(MaterialHandle p_handle, uint32 p_index, const SharedTexture &p_texture) -> void
	{
		MaterialData *data{getData(p_handle)};
		TST_PERMA_ASSERT(data);
		data->textureRefs[p_index] = p_texture;
	}

	auto MaterialManager::update(uint32 p_frame_index) -> void
	{
		for (uint32 i{0u}; i < m_pool.getSize(); ++i)
		{
			auto &data{m_pool._data[i]};

			if (m_pool._alive[i] && data.framesDirty > 0)
			{
				--data.framesDirty;
				m_device->uploadBufferData(m_materialBuffers[p_frame_index], data.data.data(), data.allocationSize, data.allocationOffset);
			}
		}
	}

	auto MaterialManager::_destroyMaterialData(const MaterialData &p_material_data, VmaVirtualBlock p_virtual_block) -> void
	{
		if (p_material_data.virtualAllocation)
			vmaVirtualFree(p_virtual_block, p_material_data.virtualAllocation);
	}
}
