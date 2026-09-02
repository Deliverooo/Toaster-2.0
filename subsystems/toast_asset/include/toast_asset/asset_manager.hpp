#pragma once

#include <filesystem>

#include "toast_gpu/resource_manager.hpp"

#include "mesh.hpp"

namespace toaster::asset
{
	struct TST_ASSET_API AssetManagerDesc
	{
		uint64 maxMaterialGPUPoolSizeBytes{10u * 1028u * 1028u}; // Max byte size for the GPU material SSBOs (default 10MB)

		// Just the sizes of the global static mesh buffers
		uint64 staticMeshVertexBufferSize{sizeof(StaticMeshVertex) * 5u * 1028u * 1028u};
		uint64 staticMeshIndexBufferSize{sizeof(uint32) * 5u * 1028u * 1028u};

		uint32 maxFramesInFlight{3u}; // Used for per-asset updates every frame
	};

	class TST_ASSET_API AssetManager
	{
		TST_REGISTER_DEPENDENCY(gpu::ResourceManager, ResourceManager, resourceManager)
	public:
		AssetManager(gpu::ResourceManager &p_resource_manager, const AssetManagerDesc &p_desc);
		~AssetManager();

		auto performGarbageCollection(uint64 p_current_timeline_value) -> void;
		auto update(uint32 p_frame_index) -> void; // Similar, but for material updations

		#pragma region textures

		auto loadTextureFromFile(gpu::CommandList &p_cmd, const std::filesystem::path &p_path) -> TextureAssetHandle;
		auto destroyTexture(TextureAssetHandle p_handle) -> void;

		auto getTexture(TextureAssetHandle p_handle) -> TextureAssetData * { return m_textureAssetPool.getData(p_handle); }
		auto getTexture(TextureAssetHandle p_handle) const -> const TextureAssetData * { return m_textureAssetPool.getData(p_handle); }

		#pragma endregion

		#pragma region materials

		auto createMaterial(uint64 p_size_bytes) -> MaterialAssetHandle;
		auto destroyMaterial(MaterialAssetHandle p_handle) -> void;

		auto getMaterial(MaterialAssetHandle p_handle) -> MaterialAssetData * { return m_materialAssetPool.getData(p_handle); }
		auto getMaterial(MaterialAssetHandle p_handle) const -> const MaterialAssetData * { return m_materialAssetPool.getData(p_handle); }

		auto getMaterialDeviceAddress(MaterialAssetHandle p_handle, uint32 p_frame_index) const -> vk::DeviceAddress;

		auto markMaterialDirty(MaterialAssetHandle p_handle) -> void;

		template<typename Type>
		auto setField(MaterialAssetHandle p_handle, uint32 p_byte_offset, const Type &p_data) -> void
		{
			MaterialAssetData *data{m_materialAssetPool.getData(p_handle)};
			TST_ASSERT(data);
			TST_ASSERT(p_byte_offset + sizeof(Type) <= data->data.size());

			std::memcpy(data->data.data() + p_byte_offset, &p_data, sizeof(Type));

			markMaterialDirty(p_handle);
		}

		auto setTextureRef(MaterialAssetHandle p_handle, uint32 p_index,  gpu::TextureHandle p_texture) -> void;

		#pragma endregion

		#pragma region static meshes

		// TODO: Mesh importer
		[[nodiscard]] auto createStaticMesh(gpu::CommandList &              p_cmd, const std::vector<StaticMeshVertex> &p_vertices, const std::vector<uint32> &p_indices,
											const std::vector<SubmeshData> &p_submeshes) -> StaticMeshAssetHandle;
		auto destroyStaticMesh(StaticMeshAssetHandle p_handle) -> void;

		auto getStaticMeshData(StaticMeshAssetHandle p_handle) -> StaticMeshAssetData * { return m_staticMeshPool.getData(p_handle); }
		auto getStaticMeshData(StaticMeshAssetHandle p_handle) const -> const StaticMeshAssetData * { return m_staticMeshPool.getData(p_handle); }

		auto getStaticMeshGlobalVertexBuffer() const -> gpu::BufferHandle { return m_staticMeshVertexBuffer; }
		auto getStaticMeshGlobalIndexBuffer() const -> gpu::BufferHandle { return m_staticMeshIndexBuffer; }

		#pragma endregion

	private:
		auto _destroyTextureAsset(TextureAssetData &p_data) -> void;
		auto _destroyMaterialAsset(MaterialAssetData &p_data) -> void;
		auto _destroyStaticMeshAsset(StaticMeshAssetData &p_data) -> void;

		gpu::ResourcePool<TextureAssetTag, TextureAssetData> m_textureAssetPool;

		uint32 m_maxFramesInFlight{3u};

		#pragma region materials
		gpu::ResourcePool<MaterialAssetTag, MaterialAssetData> m_materialAssetPool;
		VmaVirtualBlock                                        m_materialVirtualBlock{nullptr};
		std::vector<std::vector<MaterialAssetHandle> >         m_dirtyMaterials;

		std::vector<gpu::BufferHandle> m_materialBuffers;
		#pragma endregion

		#pragma region static meshes

		gpu::ResourcePool<StaticMeshAssetTag, StaticMeshAssetData> m_staticMeshPool;

		gpu::BufferHandle m_staticMeshVertexBuffer;
		gpu::BufferHandle m_staticMeshIndexBuffer;

		VmaVirtualBlock m_staticMeshVertexBufferBlock{nullptr};
		VmaVirtualBlock m_staticMeshIndexBufferBlock{nullptr};

		#pragma endregion
	};
}
