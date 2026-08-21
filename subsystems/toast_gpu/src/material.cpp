#include "toast_gpu/material.hpp"

namespace toaster::gpu
{
	MaterialManager::MaterialManager(BufferManager &p_buffer_manager, TextureManager &p_texture_manager, const RefPtr<MaterialStructMapping> &p_struct_mapping,
									 uint32         p_max_materials) : m_bufferManager(&p_buffer_manager), m_textureManager(&p_texture_manager),
																	   m_structMapping(p_struct_mapping), m_maxMaterials(p_max_materials)
	{
		m_pool.reserve(p_max_materials);
		m_pool.setUserData(this);
		m_pool.setDestroyCallback(+[](void *p_user_data, MaterialHandle p_handle) -> void
		{
			auto ts{static_cast<MaterialManager *>(p_user_data)};
			delete[] ts->m_pool._data[p_handle.id].data;
			ts->m_pool._data[p_handle.id].data = nullptr;
		});

		BufferDesc buffer_desc{};
		buffer_desc.size             = m_maxMaterials * m_structMapping->size;
		buffer_desc.usageFlags       = vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress;
		buffer_desc.memoryProperties = EMemoryProperties::eHostVisibleCoherent;

		m_materialBuffers.resize(3u);
		for (auto &buffer: m_materialBuffers)
			buffer = m_bufferManager->createBuffer(buffer_desc);
	}

	MaterialManager::~MaterialManager()
	{
		for (auto &mat: m_pool._data)
		{
			if (mat.data)
				delete[] mat.data;
		}

		for (auto &buffer: m_materialBuffers)
			m_bufferManager->destroyBuffer(buffer);
	}

	auto MaterialManager::createMaterial() -> MaterialHandle
	{
		const MaterialHandle out_handle{m_pool.create({})};
		m_pool._data[out_handle.id].data = new uint8[m_structMapping->size];
		return out_handle;
	}

	auto MaterialManager::getImage(MaterialHandle p_handle, const String &p_name) -> SharedTexture
	{
		if (!m_pool.isValid(p_handle))
			return {};
		return m_pool._data[p_handle.id].imageRefs.at(p_name);
	}

	auto MaterialManager::markMaterialDirty(MaterialHandle p_handle) -> void
	{
		if (!m_pool.isValid(p_handle))
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
				m_bufferManager->uploadDirect(m_materialBuffers[p_frame_index], entry.data, m_structMapping->size, m_structMapping->size * i);
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
