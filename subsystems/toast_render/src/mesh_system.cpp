#include "toast_render/mesh_system.hpp"

#include "toast_gpu/command_list.hpp"

namespace toaster::rd
{
	MeshSystem::MeshSystem(gpu::Device &p_device, MaterialSystem &p_material_system, const MeshSystemDesc &p_desc)
		: m_device(&p_device), m_materialSystem(&p_material_system)
	{
		gpu::BufferDesc vertex_buffer_desc{};
		vertex_buffer_desc.size = p_desc.staticMeshVertexBufferSize;
		vertex_buffer_desc.usageFlags = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress;
		vertex_buffer_desc.memoryProperties = gpu::EMemoryProperties::eDeviceLocal;
		m_staticMeshVertexBuffer = m_device->createBuffer(vertex_buffer_desc);

		gpu::BufferDesc index_buffer_desc{};
		index_buffer_desc.size             = p_desc.staticMeshIndexBufferSize;
		index_buffer_desc.usageFlags       = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer;
		index_buffer_desc.memoryProperties = gpu::EMemoryProperties::eDeviceLocal;
		m_staticMeshIndexBuffer            = m_device->createBuffer(index_buffer_desc);

		VmaVirtualBlockCreateInfo vertex_buffer_virtual_block_create_info{};
		vertex_buffer_virtual_block_create_info.size = p_desc.staticMeshVertexBufferSize;
		vmaCreateVirtualBlock(&vertex_buffer_virtual_block_create_info, &m_staticMeshVertexBufferBlock);

		VmaVirtualBlockCreateInfo index_buffer_virtual_block_create_info{};
		index_buffer_virtual_block_create_info.size = p_desc.staticMeshIndexBufferSize;
		vmaCreateVirtualBlock(&index_buffer_virtual_block_create_info, &m_staticMeshIndexBufferBlock);
	}

	MeshSystem::~MeshSystem()
	{
		m_device->submitDeletion([ vertex_block = m_staticMeshVertexBufferBlock, index_block = m_staticMeshIndexBufferBlock]() mutable noexcept -> void
		{
			vmaDestroyVirtualBlock(vertex_block);
			vmaDestroyVirtualBlock(index_block);
		});

		// Already deferred
		// m_device->releaseBuffer(m_staticMeshIndexBuffer);
		// m_device->releaseBuffer(m_staticMeshVertexBuffer);
	}

	auto MeshSystem::createStaticMesh(gpu::CommandList &              p_cmd, const std::vector<StaticMeshVertex> &p_vertices, const std::vector<uint32> &p_indices,
									  const std::vector<SubmeshData> &p_submeshes) -> Ref<MeshSystem, StaticMeshHandle>
	{
		StaticMeshData out_data{};
		out_data.submeshes = p_submeshes;

		const uint64 vertex_buffer_size{p_vertices.size() * sizeof(StaticMeshVertex)};
		const uint64 index_buffer_size{p_indices.size() * sizeof(uint32)};

		gpu::BufferRef staging_buffer{m_device->createBuffer(gpu::BufferDesc::staging(vertex_buffer_size + index_buffer_size))};

		m_device->uploadBufferData(staging_buffer.get(), p_vertices.data(), vertex_buffer_size, 0u);
		m_device->uploadBufferData(staging_buffer.get(), p_indices.data(), index_buffer_size, vertex_buffer_size);

		VmaVirtualAllocationCreateInfo vertex_virtual_allocation_create_info{};
		vertex_virtual_allocation_create_info.size      = vertex_buffer_size;
		vertex_virtual_allocation_create_info.alignment = alignof(StaticMeshVertex);
		vmaVirtualAllocate(m_staticMeshVertexBufferBlock, &vertex_virtual_allocation_create_info, &out_data.vertexBufferAllocation, &out_data.vertexBufferByteOffset);

		VmaVirtualAllocationCreateInfo index_virtual_allocation_create_info{};
		index_virtual_allocation_create_info.size      = index_buffer_size;
		index_virtual_allocation_create_info.alignment = alignof(uint32);
		vmaVirtualAllocate(m_staticMeshIndexBufferBlock, &index_virtual_allocation_create_info, &out_data.indexBufferAllocation, &out_data.indexBufferByteOffset);

		p_cmd.copyBuffer(staging_buffer.get(), m_staticMeshVertexBuffer.get(), vertex_buffer_size, 0u, out_data.vertexBufferByteOffset);
		p_cmd.copyBuffer(staging_buffer.get(), m_staticMeshIndexBuffer.get(), index_buffer_size, vertex_buffer_size, out_data.indexBufferByteOffset);

		// m_device->releaseBuffer(staging_buffer);

		StaticMeshHandle out_handle{m_staticMeshPool.create(out_data)};
		m_staticMeshPool.incRef(out_handle); // Initial ref count must be 1
		return Ref<MeshSystem, StaticMeshHandle>{this, out_handle};
	}

	auto MeshSystem::_destroyStaticMesh(StaticMeshData *p_data) -> void
	{
		if (!p_data)
			return;

		p_data->submeshes.clear();

		// I have to copy the data because the data pointer is only (technically) valid for the scope of the deletion.
		m_device->submitDeletion([vertex_block = m_staticMeshVertexBufferBlock, index_block = m_staticMeshIndexBufferBlock, data = *p_data]() mutable noexcept -> void
		{
			vmaVirtualFree(vertex_block, data.vertexBufferAllocation);
			vmaVirtualFree(index_block, data.indexBufferAllocation);
		});
	}
}
