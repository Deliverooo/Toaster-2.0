#pragma once

#include "material_manager.hpp"

namespace toaster::gpu
{
	struct TST_GPU_API StaticMeshVertex
	{
		XMFLOAT3 position;
		XMFLOAT3 normal;
		XMFLOAT2 texCoord;
	};

	struct TST_GPU_API DynamicMeshVertex
	{
		XMFLOAT3 position;
		XMFLOAT3 normal;
		XMFLOAT2 texCoord;
		//Something to do with bone weights...
	};

	struct TST_GPU_API SubmeshData
	{
		uint32 indexOffset{0u};
		int32  vertexOffset{0}; // Apparently, Vulkan wants the vertex offset as an int and not a uint.
		uint32 indexCount{0u};

		uint32 materialIndex{0u};
	};

	struct TST_GPU_API StaticMeshData
	{
		VmaVirtualAllocation vertexBufferAllocation{nullptr};
		VmaVirtualAllocation indexBufferAllocation{nullptr};

		vk::DeviceSize vertexBufferOffset{0u};
		vk::DeviceSize indexBufferOffset{0u};

		std::vector<SubmeshData>    submeshes;
		std::vector<SharedMaterial> materials;
	};

	struct TST_GPU_API DynamicMeshData
	{
		// I ain't doin allat
	};

	TST_DECLARE_HANDLE(StaticMesh);
	TST_DECLARE_HANDLE(DynamicMesh);

	class TST_GPU_API MeshManager
	{
		TST_REGISTER_DEPENDENCY(Device, Device, device)
		TST_REGISTER_DEPENDENCY(MaterialManager, MaterialManager, materialManager)
	public:
		using StaticMeshPoolType = Pool<StaticMeshTag, StaticMeshData>;

		MeshManager(Device &p_gpu_ctx,  MaterialManager &p_material_manager, uint64 p_static_mesh_vertex_buffer_size_bytes,
					uint64      p_static_mesh_index_buffer_size_bytes);
		~MeshManager();

		MeshManager(const MeshManager &)            = delete;
		MeshManager(MeshManager &&)                 = delete;
		MeshManager &operator=(const MeshManager &) = delete;
		MeshManager &operator=(MeshManager &&)      = delete;

		[[nodiscard]] auto createStaticMesh(CommandList& p_cmd, const std::vector<StaticMeshVertex> &p_vertices, const std::vector<uint32> &p_indices,
											const std::vector<SubmeshData> &p_submeshes, const std::vector<SharedMaterial> &p_materials) -> StaticMeshHandle;
		auto destroyStaticMesh(StaticMeshHandle p_handle) -> void { m_staticMeshPool.destroy(p_handle); }

		auto getStaticMeshData(StaticMeshHandle p_handle) const -> const StaticMeshData * { return m_staticMeshPool.getData(p_handle); }
		auto getStaticMeshData(StaticMeshHandle p_handle) -> StaticMeshData * { return m_staticMeshPool.getData(p_handle); }

		auto getStaticMeshGlobalVertexBuffer() const -> vk::Buffer { return m_device->getBufferData(m_staticMeshVertexBuffer)->buffer; }
		auto getStaticMeshGlobalIndexBuffer() const -> vk::Buffer { return m_device->getBufferData(m_staticMeshIndexBuffer)->buffer; }

	private:
		// All the required dependencies to destroy a static mesh. May be called after this class is destroyed due to the deletion queue
		static auto _destroyStaticMeshData(const StaticMeshData &p_static_mesh_data, VmaVirtualBlock p_vertex_buffer_block, VmaVirtualBlock p_index_buffer_block) -> void;

		StaticMeshPoolType m_staticMeshPool;

		BufferHandle m_staticMeshVertexBuffer;
		BufferHandle m_staticMeshIndexBuffer;

		VmaVirtualBlock m_staticMeshVertexBufferBlock{nullptr};
		VmaVirtualBlock m_staticMeshIndexBufferBlock{nullptr};
	};
}
