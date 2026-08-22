#include "toast_gpu/material.hpp"

namespace toaster::gpu
{
	MaterialManager::MaterialManager(BufferManager &        p_buffer_manager, uint32 p_max_pool_size_bytes, void *p_user_data,
									 const DestroyCallback &p_destroy_callback) : m_bufferManager(&p_buffer_manager), m_maxPoolSize(p_max_pool_size_bytes),
																				  m_userData(p_user_data), m_destroyCallback(p_destroy_callback)
	{
		BufferDesc material_buffer_desc{};
		material_buffer_desc.size             = m_maxPoolSize;
		material_buffer_desc.usageFlags       = vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer;
		material_buffer_desc.memoryProperties = EMemoryProperties::eHostVisibleCoherent;

		m_materialBuffers.resize(3u);

		for (auto &buffer: m_materialBuffers)
			buffer = m_bufferManager->createBuffer(material_buffer_desc);

		m_pool.setUserData(this);
		m_pool.setDestroyCallback(+[](void *p_user_data, MaterialHandle p_handle) -> void
		{
			auto ts{static_cast<MaterialManager *>(p_user_data)};

			MaterialData *material_data{&ts->m_pool._data[p_handle.id]};

			if (ts->m_destroyCallback)
				ts->m_destroyCallback(ts->m_userData, p_handle);
			else
				ts->destroyData(material_data);
		});
	}

	MaterialManager::~MaterialManager()
	{
		for (auto &buffer: m_materialBuffers)
			m_bufferManager->destroyBuffer(buffer);

		// For safety...
		for (uint32 i{0u}; i < m_pool.getSize(); ++i)
		{
			if (m_pool._alive[i])
				destroyData(&m_pool._data[i]);
		}
	}

	auto MaterialManager::createMaterial(uint32 p_size_bytes) -> MaterialHandle
	{
		MaterialData out_data{};
		out_data.data.resize(p_size_bytes, 0);

		uint32 aligned_size{TST_ALIGN(p_size_bytes, 16)};
		out_data.allocationSize   = aligned_size;
		out_data.allocationOffset = m_currentAllocationOffset;
		m_currentAllocationOffset += aligned_size;
		out_data.framesDirty      = 3u;

		return m_pool.create(out_data);
	}

	auto MaterialManager::createSharedMaterial(uint32 p_size_bytes) -> SharedMaterial
	{
		return SharedMaterial{createMaterial(p_size_bytes), &m_pool};
	}

	auto MaterialManager::destroyData(MaterialData *p_data) -> void
	{
		p_data->data.clear();
		p_data->textureRefs.fill({});
	}

	auto MaterialManager::getMaterialDeviceAddress(MaterialHandle p_handle, uint32 p_frame_index) const -> vk::DeviceAddress
	{
		const MaterialData *data{getData(p_handle)};
		TST_PERMA_ASSERT(data);

		return m_bufferManager->getBufferDeviceAddress(m_materialBuffers[p_frame_index]) + data->allocationOffset;
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
				m_bufferManager->uploadDirect(m_materialBuffers[p_frame_index], data.data.data(), data.allocationSize, data.allocationOffset);
			}
		}
	}
}
