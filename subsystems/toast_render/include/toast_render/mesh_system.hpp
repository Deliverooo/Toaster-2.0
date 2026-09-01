#pragma once

#include "toast_render.hpp"
#include "material_system.hpp"

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

		MaterialRef material{};
	};

	struct TST_RENDER_API StaticMeshData
	{
		VmaVirtualAllocation vertexBufferAllocation{nullptr};
		VmaVirtualAllocation indexBufferAllocation{nullptr};

		// These are BYTE offsets... :(
		uint64 vertexBufferByteOffset{0u};
		uint64 indexBufferByteOffset{0u};

		auto getVertexBufferOffset() const -> uint32 { return static_cast<uint32>(vertexBufferByteOffset) / sizeof(StaticMeshVertex); }
		auto getIndexBufferOffset() const -> uint32 { return static_cast<uint32>(indexBufferByteOffset) / sizeof(uint32); }

		std::vector<SubmeshData> submeshes;
	};

	TST_DECLARE_HANDLE(StaticMesh);
	TST_DECLARE_REF(StaticMesh);

	struct TST_RENDER_API MeshSystemDesc
	{
		// Just the sizes of the global static mesh buffers
		uint64 staticMeshVertexBufferSize{sizeof(StaticMeshVertex) * 5u * 1028u * 1028u};
		uint64 staticMeshIndexBufferSize{sizeof(uint32) * 5u * 1028u * 1028u};
	};

	class TST_RENDER_API MeshSystem
	{
		TST_REGISTER_DEPENDENCY(gpu::Device, Device, device)
		TST_REGISTER_DEPENDENCY(MaterialSystem, MaterialSystem, materialSystem)
	public:
		MeshSystem(gpu::Device &p_device, MaterialSystem &p_material_system, const MeshSystemDesc &p_desc);
		~MeshSystem();

		MeshSystem(const MeshSystem &)            = delete;
		MeshSystem(MeshSystem &&)                 = delete;
		MeshSystem &operator=(const MeshSystem &) = delete;
		MeshSystem &operator=(MeshSystem &&)      = delete;

		[[nodiscard]] auto createStaticMesh(gpu::CommandList &              p_cmd, const std::vector<StaticMeshVertex> &p_vertices, const std::vector<uint32> &p_indices,
											const std::vector<SubmeshData> &p_submeshes) -> StaticMeshRef;

		auto getStaticMeshGlobalVertexBuffer() const -> gpu::BufferHandle { return m_staticMeshVertexBuffer.get(); }
		auto getStaticMeshGlobalIndexBuffer() const -> gpu::BufferHandle { return m_staticMeshIndexBuffer.get(); }

	private:
		// All the required dependencies to destroy a static mesh. May be called after this class is destroyed due to the deletion queue
		auto _destroyStaticMesh(StaticMeshData *p_data) -> void;

		ResourceManager<StaticMeshTag, StaticMeshData> m_staticMeshResourceManager;

		gpu::BufferRef m_staticMeshVertexBuffer;
		gpu::BufferRef m_staticMeshIndexBuffer;

		VmaVirtualBlock m_staticMeshVertexBufferBlock{nullptr};
		VmaVirtualBlock m_staticMeshIndexBufferBlock{nullptr};
	};
}
