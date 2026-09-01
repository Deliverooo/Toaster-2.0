#include "toast_render/material_system.hpp"

namespace toaster::rd
{
	MaterialSystem::MaterialSystem(gpu::Device &p_device, uint32 p_max_pool_size_bytes, uint32 p_frames_in_flight) : m_device(&p_device),
																													 m_maxPoolSize(p_max_pool_size_bytes),
																													 m_framesInFlight(p_frames_in_flight)
	{
		VmaVirtualBlockCreateInfo block_create_info{};
		block_create_info.size = m_maxPoolSize;
		vmaCreateVirtualBlock(&block_create_info, &m_virtualBlock);

		gpu::BufferDesc material_buffer_desc{};
		material_buffer_desc.size             = m_maxPoolSize;
		material_buffer_desc.usageFlags       = vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer;
		material_buffer_desc.memoryProperties = gpu::EMemoryProperties::eHostVisibleCoherent;

		m_materialBuffers.resize(m_framesInFlight);
		for (auto &buffer: m_materialBuffers)
			buffer = m_device->createBuffer(material_buffer_desc);

		m_materialResourceManager.setDeleterUserData(this);
		m_materialResourceManager.setDeleter(+[](void *p_user_data, MaterialData *p_data) mutable noexcept -> void
		{
			auto ts{static_cast<MaterialSystem *>(p_user_data)};
			ts->_destroyMaterial(p_data);
		});
	}

	MaterialSystem::~MaterialSystem()
	{
		m_materialBuffers.clear();

		m_device->submitDeletion([virtual_block = m_virtualBlock] mutable noexcept -> void
		{
			vmaDestroyVirtualBlock(virtual_block);
		});
	}

	auto MaterialSystem::createMaterial(uint32 p_size_bytes) -> MaterialRef
	{
		MaterialData out_data{};
		out_data.data.resize(p_size_bytes, 0);

		VmaVirtualAllocationCreateInfo virtual_allocation_create_info{};
		virtual_allocation_create_info.size      = p_size_bytes;
		virtual_allocation_create_info.alignment = 16u;

		vk::Result res{vmaVirtualAllocate(m_virtualBlock, &virtual_allocation_create_info, &out_data.virtualAllocation, &out_data.allocationOffset)};
		TST_PERMA_ASSERT(res == vk::Result::eSuccess);

		out_data.allocationSize = p_size_bytes;
		out_data.framesDirty    = m_framesInFlight;

		return m_materialResourceManager.create(out_data);
	}

	auto MaterialSystem::getMaterialDeviceAddress(MaterialHandle p_handle, uint32 p_frame_index) const -> vk::DeviceAddress
	{
		const MaterialData *data{m_materialResourceManager.getData(p_handle)};
		TST_ASSERT(data);

		return m_materialBuffers[p_frame_index]->address + data->allocationOffset;
	}

	auto MaterialSystem::setTextureRef(MaterialHandle p_handle, uint32 p_index, const gpu::TextureRef &p_texture) -> void
	{
		MaterialData *data{m_materialResourceManager.getData(p_handle)};
		TST_ASSERT(data);

		data->textureRefs[p_index] = p_texture;
	}

	auto MaterialSystem::update(uint32 p_frame_index) -> void
	{
		m_materialResourceManager.forEachAlive([this, p_frame_index](MaterialData &p_data) -> void
		{
			if (p_data.framesDirty > 0)
			{
				--p_data.framesDirty;
				m_device->uploadBufferData(m_materialBuffers[p_frame_index].get(), p_data.data.data(), p_data.allocationSize, p_data.allocationOffset);
			}
		});
	}

	auto MaterialSystem::_destroyMaterial(MaterialData *p_data) -> void
	{
		p_data->data.clear(); // We don't need to defer this

		for (auto &tex: p_data->textureRefs)
			tex.reset();

		// I have to copy the data because the data pointer is only (technically) valid for the scope of the deletion.
		m_device->submitDeletion([virtual_block = m_virtualBlock, data = *p_data] mutable noexcept -> void
		{
			if (data.virtualAllocation)
				vmaVirtualFree(virtual_block, data.virtualAllocation);
		});
	}
}
