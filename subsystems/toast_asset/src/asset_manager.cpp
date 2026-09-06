#include "toast_asset/asset_manager.hpp"

#include "toast_asset/texture.hpp"

namespace toaster::asset
{
	AssetManager::AssetManager(gpu::ResourceManager &p_resource_manager, const AssetManagerDesc &p_desc) : m_resourceManager(&p_resource_manager),
																										   m_maxFramesInFlight(p_desc.maxFramesInFlight)
	{
		#pragma region materials

		VmaVirtualBlockCreateInfo block_create_info{};
		block_create_info.size = p_desc.maxMaterialGPUPoolSizeBytes;
		vmaCreateVirtualBlock(&block_create_info, &m_materialVirtualBlock);

		gpu::BufferDesc material_buffer_desc{};
		material_buffer_desc.size             = p_desc.maxMaterialGPUPoolSizeBytes;
		material_buffer_desc.usageFlags       = vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer;
		material_buffer_desc.memoryProperties = gpu::EMemoryProperties::eHostVisibleCoherent;

		m_materialBuffers.resize(m_maxFramesInFlight);
		for (auto &buffer: m_materialBuffers)
			buffer = m_resourceManager->createBuffer(material_buffer_desc);

		m_dirtyMaterials.resize(m_maxFramesInFlight);

		#pragma endregion

		#pragma region static meshes

		gpu::BufferDesc vertex_buffer_desc{};
		vertex_buffer_desc.size = p_desc.staticMeshVertexBufferSize;
		vertex_buffer_desc.usageFlags = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress;
		vertex_buffer_desc.memoryProperties = gpu::EMemoryProperties::eDeviceLocal;
		m_staticMeshVertexBuffer = m_resourceManager->createBuffer(vertex_buffer_desc);

		gpu::BufferDesc index_buffer_desc{};
		index_buffer_desc.size             = p_desc.staticMeshIndexBufferSize;
		index_buffer_desc.usageFlags       = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer;
		index_buffer_desc.memoryProperties = gpu::EMemoryProperties::eDeviceLocal;
		m_staticMeshIndexBuffer            = m_resourceManager->createBuffer(index_buffer_desc);

		VmaVirtualBlockCreateInfo vertex_buffer_virtual_block_create_info{};
		vertex_buffer_virtual_block_create_info.size = p_desc.staticMeshVertexBufferSize;
		vmaCreateVirtualBlock(&vertex_buffer_virtual_block_create_info, &m_staticMeshVertexBufferBlock);

		VmaVirtualBlockCreateInfo index_buffer_virtual_block_create_info{};
		index_buffer_virtual_block_create_info.size = p_desc.staticMeshIndexBufferSize;
		vmaCreateVirtualBlock(&index_buffer_virtual_block_create_info, &m_staticMeshIndexBufferBlock);

		#pragma endregion
	}

	AssetManager::~AssetManager()
	{
		m_staticMeshPool.purgeAll([this](StaticMeshAssetData &p_data) mutable-> void { _destroyStaticMeshAsset(p_data); });

		vmaDestroyVirtualBlock(m_staticMeshVertexBufferBlock);
		vmaDestroyVirtualBlock(m_staticMeshIndexBufferBlock);

		m_textureAssetPool.purgeAll([this](TextureAssetData &p_data) mutable-> void { _destroyTextureAsset(p_data); });
		m_materialAssetPool.purgeAll([this](MaterialAssetData &p_data) mutable-> void { _destroyMaterialAsset(p_data); });
	}

	auto AssetManager::performGarbageCollection(uint64 p_current_timeline_value) -> void
	{
		m_textureAssetPool.cleanupDeletions(p_current_timeline_value, [this](TextureAssetData &p_data) mutable-> void { _destroyTextureAsset(p_data); });
		m_materialAssetPool.cleanupDeletions(p_current_timeline_value, [this](MaterialAssetData &p_data) mutable-> void { _destroyMaterialAsset(p_data); });
		m_staticMeshPool.cleanupDeletions(p_current_timeline_value, [this](StaticMeshAssetData &p_data) mutable-> void { _destroyStaticMeshAsset(p_data); });
	}

	auto AssetManager::update(uint32 p_frame_index) -> void
	{
		auto &dirty_list{m_dirtyMaterials[p_frame_index]};
		if (dirty_list.empty())
			return;

		for (const auto handle: dirty_list)
		{
			MaterialAssetData *material_data{(m_materialAssetPool.getData(handle))};
			m_resourceManager->uploadBufferData(m_materialBuffers[p_frame_index], material_data->data.data(), material_data->allocationSize,
												material_data->allocationOffset);
		}

		dirty_list.clear();
	}

	auto AssetManager::loadTextureFromFile(gpu::CommandList &p_cmd, const std::filesystem::path &p_path) -> TextureAssetHandle
	{
		TST_ASSERT(std::filesystem::exists(p_path));

		gpu::TextureDesc texture_desc{};
		texture_desc.layerCount        = 1u;
		texture_desc.mipLevels         = 1u;
		texture_desc.type              = vk::ImageType::e2D;
		texture_desc.usageFlags        = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
		texture_desc.createDescriptors = true;
		texture_desc.extent.depth      = 1u;
		DataBuffer texture_data{loadTextureIntoBuffer(p_path.string().c_str(), texture_desc.format, texture_desc.extent.width, texture_desc.extent.height)};

		gpu::TextureHandle gpu_texture{m_resourceManager->createTexture(texture_desc)};
		p_cmd.transitionTextureLayout(*m_resourceManager, gpu_texture, vk::ImageLayout::eTransferDstOptimal);

		gpu::BufferHandle buff{m_resourceManager->createBuffer(gpu::BufferDesc::staging(texture_data.size()))};
		m_resourceManager->uploadBufferData(buff, texture_data.data(), texture_data.size());
		p_cmd.copyBufferToTexture(*m_resourceManager, buff, gpu_texture);
		m_resourceManager->destroyBuffer(buff);
		texture_data.release();

		p_cmd.transitionTextureLayout(*m_resourceManager, gpu_texture, vk::ImageLayout::eShaderReadOnlyOptimal);

		TextureAssetData out_data{};
		out_data.uri     = p_path.string();
		out_data.texture = gpu_texture;
		out_data.size    = {texture_desc.extent.width, texture_desc.extent.height};
		return m_textureAssetPool.emplace(out_data);
	}

	auto AssetManager::destroyTexture(TextureAssetHandle p_handle) -> void
	{
		m_textureAssetPool.destroy(p_handle, m_resourceManager->getGlobalTimelineValue());
	}

	auto AssetManager::createMaterial(uint64 p_size_bytes) -> MaterialAssetHandle
	{
		MaterialAssetData out_data{};
		out_data.data.resize(p_size_bytes, 0);

		VmaVirtualAllocationCreateInfo virtual_allocation_create_info{};
		virtual_allocation_create_info.size      = p_size_bytes;
		virtual_allocation_create_info.alignment = 16u;

		vk::Result res{vmaVirtualAllocate(m_materialVirtualBlock, &virtual_allocation_create_info, &out_data.virtualAllocation, &out_data.allocationOffset)};
		TST_PERMA_ASSERT(res == vk::Result::eSuccess);

		out_data.allocationSize = p_size_bytes;

		MaterialAssetHandle out_handle{m_materialAssetPool.emplace(out_data)};
		markMaterialDirty(out_handle);
		return out_handle;
	}

	auto AssetManager::destroyMaterial(MaterialAssetHandle p_handle) -> void
	{
		m_materialAssetPool.destroy(p_handle, m_resourceManager->getGlobalTimelineValue());
	}

	auto AssetManager::getMaterialDeviceAddress(MaterialAssetHandle p_handle, uint32 p_frame_index) const -> vk::DeviceAddress
	{
		const MaterialAssetData *data{m_materialAssetPool.getData(p_handle)};
		TST_ASSERT(data);

		return m_resourceManager->getBufferData(m_materialBuffers[p_frame_index])->address + data->allocationOffset;
	}

	auto AssetManager::markMaterialDirty(MaterialAssetHandle p_handle) -> void
	{
		for (uint32 i{0u}; i < m_maxFramesInFlight; ++i)
		{
			if (!std::ranges::contains(m_dirtyMaterials[i], p_handle))
				m_dirtyMaterials[i].push_back(p_handle);
		}
	}

	auto AssetManager::setTextureRef(MaterialAssetHandle p_handle, uint32 p_index, TextureAssetHandle p_texture) -> void
	{
		MaterialAssetData *data{m_materialAssetPool.getData(p_handle)};
		TST_ASSERT(data);

		data->textureRefs[p_index] = p_texture;
	}

	auto AssetManager::createStaticMesh(gpu::CommandList &              p_cmd, const std::vector<StaticMeshVertex> &p_vertices, const std::vector<uint32> &p_indices,
										const std::vector<SubmeshData> &p_submeshes) -> StaticMeshAssetHandle
	{
		StaticMeshAssetData out_data{};
		out_data.submeshes = p_submeshes;

		const uint64 vertex_buffer_size{p_vertices.size() * sizeof(StaticMeshVertex)};
		const uint64 index_buffer_size{p_indices.size() * sizeof(uint32)};

		gpu::BufferHandle staging_buffer{m_resourceManager->createBuffer(gpu::BufferDesc::staging(vertex_buffer_size + index_buffer_size))};

		m_resourceManager->uploadBufferData(staging_buffer, p_vertices.data(), vertex_buffer_size, 0u);
		m_resourceManager->uploadBufferData(staging_buffer, p_indices.data(), index_buffer_size, vertex_buffer_size);

		VmaVirtualAllocationCreateInfo vertex_virtual_allocation_create_info{};
		vertex_virtual_allocation_create_info.size      = vertex_buffer_size;
		vertex_virtual_allocation_create_info.alignment = alignof(StaticMeshVertex);
		vmaVirtualAllocate(m_staticMeshVertexBufferBlock, &vertex_virtual_allocation_create_info, &out_data.vertexBufferAllocation, &out_data.vertexBufferByteOffset);

		VmaVirtualAllocationCreateInfo index_virtual_allocation_create_info{};
		index_virtual_allocation_create_info.size      = index_buffer_size;
		index_virtual_allocation_create_info.alignment = alignof(uint32);
		vmaVirtualAllocate(m_staticMeshIndexBufferBlock, &index_virtual_allocation_create_info, &out_data.indexBufferAllocation, &out_data.indexBufferByteOffset);

		p_cmd.copyBuffer(*m_resourceManager, staging_buffer, m_staticMeshVertexBuffer, vertex_buffer_size, 0u, out_data.vertexBufferByteOffset);
		p_cmd.copyBuffer(*m_resourceManager, staging_buffer, m_staticMeshIndexBuffer, index_buffer_size, vertex_buffer_size, out_data.indexBufferByteOffset);

		return m_staticMeshPool.emplace(out_data);
	}

	auto AssetManager::destroyStaticMesh(StaticMeshAssetHandle p_handle) -> void
	{
		m_staticMeshPool.destroy(p_handle, m_resourceManager->getGlobalTimelineValue());
	}

	auto AssetManager::_destroyTextureAsset(TextureAssetData &p_data) -> void
	{
		m_resourceManager->destroyTexture(p_data.texture);
	}

	auto AssetManager::_destroyMaterialAsset(MaterialAssetData &p_data) -> void
	{
		p_data.data.clear();

		if (p_data.virtualAllocation)
			vmaVirtualFree(m_materialVirtualBlock, p_data.virtualAllocation);
	}

	auto AssetManager::_destroyStaticMeshAsset(StaticMeshAssetData &p_data) -> void
	{
		vmaVirtualFree(m_staticMeshVertexBufferBlock, p_data.vertexBufferAllocation);
		vmaVirtualFree(m_staticMeshIndexBufferBlock, p_data.indexBufferAllocation);
	}
}
