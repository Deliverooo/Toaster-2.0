#pragma once

#include "toast_render.hpp"
#include "toast_gpu/material_manager.hpp"

namespace toaster::rd
{
	struct TST_RENDER_API StaticMeshVertex
	{
		XMFLOAT3 position;
		XMFLOAT3 normal;
		XMFLOAT2 texCoord;
	};

	// struct TST_RENDER_API DynamicMeshVertex // TODO: Vertex weights / bone stuff...
	// {
	// 	XMFLOAT3 position;
	// 	XMFLOAT3 normal;
	// 	XMFLOAT2 texCoord;
	// };

	struct TST_RENDER_API SubmeshData
	{
		uint32 indexOffset{0u};
		int32  vertexOffset{0}; // Apparently, Vulkan wants the vertex offset as an int and not a uint.
		uint32 indexCount{0u};

		gpu::MaterialHandle material; // Not an index into a per-mesh material array.
	};

	struct TST_RENDER_API StaticMeshData
	{
		VmaVirtualAllocation vertexBufferAllocation{nullptr};
		VmaVirtualAllocation indexBufferAllocation{nullptr};

		uint64 vertexBufferOffset{0u};
		uint64 indexBufferOffset{0u};

		std::vector<SubmeshData> submeshes;
	};

	TST_DECLARE_HANDLE(StaticMesh);

	struct TST_RENDER_API MeshSystemDesc
	{
		// Just the sizes of the global static mesh buffers
		uint64 staticMeshVertexBufferSize{sizeof(StaticMeshVertex) * 5u * 1028u * 1028u};
		uint64 staticMeshIndexBufferSize{sizeof(uint32) * 5u * 1028u * 1028u};
	};
	class TST_RENDER_API MeshSystem
	{
		TST_REGISTER_DEPENDENCY(gpu::Device, Device, device)
		TST_REGISTER_DEPENDENCY(gpu::MaterialManager, MaterialManager, materialManager)
	public:
		MeshSystem(gpu::Device& p_device, gpu::MaterialManager& p_material_manager, const MeshSystemDesc &p_desc);
		~MeshSystem();

		MeshSystem(const MeshSystem &)            = delete;
		MeshSystem(MeshSystem &&)                 = delete;
		MeshSystem &operator=(const MeshSystem &) = delete;
		MeshSystem &operator=(MeshSystem &&)      = delete;

		// The mesh will acquire the materials
		[[nodiscard]] auto createStaticMesh(gpu::CommandList& p_cmd, const std::vector<StaticMeshVertex> &p_vertices,
			const std::vector<uint32> &p_indices, const std::vector<SubmeshData> &p_submeshes) -> StaticMeshHandle;
		auto			   acquireStaticMesh(StaticMeshHandle p_handle) -> void { m_staticMeshPool.incRef(p_handle);}
		auto               releaseStaticMesh(StaticMeshHandle p_handle) -> void { _destroyStaticMesh(m_staticMeshPool.decRef(p_handle)); }
		auto               isStaticMeshValid(StaticMeshHandle p_handle) const -> bool { return m_staticMeshPool.isValid(p_handle); }

		auto getStaticMeshData(StaticMeshHandle p_handle) const -> const StaticMeshData * { return m_staticMeshPool.getData(p_handle); }
		auto getStaticMeshData(StaticMeshHandle p_handle) -> StaticMeshData * { return m_staticMeshPool.getData(p_handle); }

		auto getStaticMeshGlobalVertexBuffer() const -> gpu::BufferHandle { return m_staticMeshVertexBuffer; }
		auto getStaticMeshGlobalIndexBuffer() const -> gpu::BufferHandle { return m_staticMeshIndexBuffer; }

	private:
		// All the required dependencies to destroy a static mesh. May be called after this class is destroyed due to the deletion queue
		auto _destroyStaticMesh(StaticMeshData *p_data) -> void;

		Pool<StaticMeshTag, StaticMeshData> m_staticMeshPool;

		gpu::BufferHandle m_staticMeshVertexBuffer;
		gpu::BufferHandle m_staticMeshIndexBuffer;

		VmaVirtualBlock m_staticMeshVertexBufferBlock{nullptr};
		VmaVirtualBlock m_staticMeshIndexBufferBlock{nullptr};
	};
}