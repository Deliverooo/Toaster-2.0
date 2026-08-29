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
	}

	MaterialManager::~MaterialManager()
	{
		for (auto &buffer: m_materialBuffers)
			m_device->releaseBuffer(buffer);

		// // Delete all remaining materials
		// for (auto material : m_materialPool.getAlive())
		// 	_destroyMaterial(material);

		m_device->submitDeletion([virtual_block = m_virtualBlock] mutable noexcept -> void
		{
			vmaDestroyVirtualBlock(virtual_block);
		});

		m_device->flushDeletionQueue(); // The previous calls to release material will reference this
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

		MaterialHandle out_handle{m_materialPool.create(out_data)};
		m_materialPool.incRef(out_handle); // Initial ref count must be 1
		return out_handle;
	}


	auto MaterialManager::getMaterialDeviceAddress(MaterialHandle p_handle, uint32 p_frame_index) const -> vk::DeviceAddress
	{
		const MaterialData *data{getMaterialData(p_handle)};
		TST_ASSERT(data);

		vk::DeviceAddress base_address{m_device->getBufferData(m_materialBuffers[p_frame_index])->address};
		return base_address + data->allocationOffset;
	}

	auto MaterialManager::setTextureRef(MaterialHandle p_handle, uint32 p_index, TextureHandle p_texture) -> void
	{
		MaterialData *data{getMaterialData(p_handle)};
		TST_ASSERT(data);

		m_device->acquireTexture(p_texture);
		data->textureRefs[p_index] = p_texture;
	}

	auto MaterialManager::update(uint32 p_frame_index) -> void
	{
		for (uint32 i{0u}; i < m_materialPool.getSize(); ++i)
		{
			auto &data{m_materialPool._data[i]};

			if (m_materialPool._alive[i] && data.framesDirty > 0)
			{
				--data.framesDirty;
				m_device->uploadBufferData(m_materialBuffers[p_frame_index], data.data.data(), data.allocationSize, data.allocationOffset);
			}
		}
	}

	auto MaterialManager::_destroyMaterial(MaterialData *p_data) -> void
	{
		if (!p_data)
			return;

		p_data->data.clear();         // We don't need to defer this

		for (const auto tex: p_data->textureRefs)
		{
			if (m_device->isTextureValid(tex))
				m_device->releaseTexture(tex);
		}

		// I have to copy the data because the data pointer is only (technically) valid for the scope of the deletion.
		m_device->submitDeletion([this, data = *p_data]() mutable noexcept -> void
		{
			if (data.virtualAllocation)
				vmaVirtualFree(m_virtualBlock, data.virtualAllocation);
		});

	}
}
