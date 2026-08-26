#include "toast_gpu/mesh_manager.hpp"

namespace toaster::gpu
{
	MeshManager::MeshManager(Device &p_gpu_ctx, BufferManager &p_buffer_manager, MaterialManager &p_material_manager, uint64 p_static_mesh_vertex_buffer_size_bytes,
							 uint64  p_static_mesh_index_buffer_size_bytes) : m_gpuCtx(&p_gpu_ctx), m_bufferManager(&p_buffer_manager),
																			  m_materialManager(&p_material_manager)
	{
		m_staticMeshPool.setUserData(this);
		m_staticMeshPool.setDestroyCallback(+[](void *p_user_data, StaticMeshHandle p_handle) -> void
		{
			auto ts{static_cast<MeshManager *>(p_user_data)};

			StaticMeshData *mesh_data{&ts->m_staticMeshPool._data[p_handle.id]};
			mesh_data->materials.clear(); // Material destruction is already queued

			ts->m_gpuCtx->submitDeletion([data = *mesh_data, vertex_block = ts->m_staticMeshVertexBufferBlock, index_block = ts->m_staticMeshIndexBufferBlock
										 ]() mutable noexcept -> void // Copy
										 {
											 _destroyStaticMeshData(data, vertex_block, index_block);
										 });
		});

		BufferDesc vertex_buffer_desc{};
		vertex_buffer_desc.size             = p_static_mesh_vertex_buffer_size_bytes;
		vertex_buffer_desc.usageFlags       = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;
		vertex_buffer_desc.memoryProperties = EMemoryProperties::eDeviceLocal;
		m_staticMeshVertexBuffer            = m_bufferManager->createBuffer(vertex_buffer_desc);

		BufferDesc index_buffer_desc{};
		index_buffer_desc.size             = p_static_mesh_index_buffer_size_bytes;
		index_buffer_desc.usageFlags       = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer;
		index_buffer_desc.memoryProperties = EMemoryProperties::eDeviceLocal;
		m_staticMeshIndexBuffer            = m_bufferManager->createBuffer(index_buffer_desc);

		VmaVirtualBlockCreateInfo vertex_buffer_virtual_block_create_info{};
		vertex_buffer_virtual_block_create_info.size = p_static_mesh_vertex_buffer_size_bytes;
		vmaCreateVirtualBlock(&vertex_buffer_virtual_block_create_info, &m_staticMeshVertexBufferBlock);

		VmaVirtualBlockCreateInfo index_buffer_virtual_block_create_info{};
		index_buffer_virtual_block_create_info.size = p_static_mesh_index_buffer_size_bytes;
		vmaCreateVirtualBlock(&index_buffer_virtual_block_create_info, &m_staticMeshIndexBufferBlock);
	}

	MeshManager::~MeshManager()
	{
		// For safety...
		for (uint32 i{0u}; i < m_staticMeshPool.getSize(); ++i)
		{
			if (m_staticMeshPool._alive[i])
			{
				StaticMeshData *mesh_data{&m_staticMeshPool._data[i]};
				mesh_data->materials.clear(); // Material destruction is already queued

				m_gpuCtx->submitDeletion([data = *mesh_data, vertex_block = m_staticMeshVertexBufferBlock, index_block = m_staticMeshIndexBufferBlock
										 ]() mutable noexcept -> void // Copy
										 {
											 _destroyStaticMeshData(data, vertex_block, index_block);
										 });
			}
		}

		m_gpuCtx->submitDeletion([ vertex_block = m_staticMeshVertexBufferBlock, index_block = m_staticMeshIndexBufferBlock]() mutable noexcept -> void
		{
			vmaDestroyVirtualBlock(vertex_block);
			vmaDestroyVirtualBlock(index_block);
		});

		// Already deferred
		m_bufferManager->destroyBuffer(m_staticMeshIndexBuffer);
		m_bufferManager->destroyBuffer(m_staticMeshVertexBuffer);
	}

	auto MeshManager::createStaticMesh(vk::CommandBuffer               p_cmd, const std::vector<StaticMeshVertex> &    p_vertices, const std::vector<uint32> &p_indices,
									   const std::vector<SubmeshData> &p_submeshes, const std::vector<SharedMaterial> &p_materials) -> StaticMeshHandle
	{
		StaticMeshData out_data{};
		out_data.submeshes = p_submeshes;
		out_data.materials = p_materials;

		const uint64 vertex_buffer_size{p_vertices.size() * sizeof(StaticMeshVertex)};
		const uint64 index_buffer_size{p_indices.size() * sizeof(uint32)};

		BufferDesc staging_buffer_desc{}; // TODO: global staging buffer
		staging_buffer_desc.size             = vertex_buffer_size + index_buffer_size;
		staging_buffer_desc.usageFlags       = vk::BufferUsageFlagBits::eTransferSrc;
		staging_buffer_desc.memoryProperties = EMemoryProperties::eHostVisibleCoherent;
		BufferHandle staging_buffer{m_bufferManager->createBuffer(staging_buffer_desc)};

		m_bufferManager->uploadDirect(staging_buffer, p_vertices.data(), vertex_buffer_size, 0u);
		m_bufferManager->uploadDirect(staging_buffer, p_indices.data(), index_buffer_size, vertex_buffer_size);

		VmaVirtualAllocationCreateInfo vertex_virtual_allocation_create_info{};
		vertex_virtual_allocation_create_info.size      = vertex_buffer_size;
		vertex_virtual_allocation_create_info.alignment = sizeof(StaticMeshVertex);
		vmaVirtualAllocate(m_staticMeshVertexBufferBlock, &vertex_virtual_allocation_create_info, &out_data.vertexBufferAllocation, &out_data.vertexBufferOffset);

		VmaVirtualAllocationCreateInfo index_virtual_allocation_create_info{};
		index_virtual_allocation_create_info.size      = index_buffer_size;
		index_virtual_allocation_create_info.alignment = sizeof(uint32);
		vmaVirtualAllocate(m_staticMeshIndexBufferBlock, &index_virtual_allocation_create_info, &out_data.indexBufferAllocation, &out_data.indexBufferOffset);

		m_bufferManager->copyBuffer(staging_buffer, m_staticMeshVertexBuffer, p_cmd, vertex_buffer_size, 0u, out_data.vertexBufferOffset);
		m_bufferManager->copyBuffer(staging_buffer, m_staticMeshIndexBuffer, p_cmd, index_buffer_size, vertex_buffer_size, out_data.indexBufferOffset);

		m_bufferManager->destroyBuffer(staging_buffer);

		return m_staticMeshPool.create(out_data);
	}

	auto MeshManager::_destroyStaticMeshData(const StaticMeshData &p_static_mesh_data, VmaVirtualBlock p_vertex_buffer_block,
											 VmaVirtualBlock       p_index_buffer_block) -> void
	{
		vmaVirtualFree(p_vertex_buffer_block, p_static_mesh_data.vertexBufferAllocation);
		vmaVirtualFree(p_index_buffer_block, p_static_mesh_data.indexBufferAllocation);
	}
}
