#include "toast_render/transform_system.hpp"

namespace toaster::rd
{
	TransformSystem::TransformSystem(gpu::ResourceManager &p_resource_manager,  uint32 p_max_transforms, uint32 p_frames_in_flight) : m_resourceManager(&p_resource_manager), m_maxTransforms(p_max_transforms)
	{
		m_freeTransformList = FreelistAllocator<uint32>{m_maxTransforms};

		gpu::BufferDesc transform_buffer_desc{};
		transform_buffer_desc.size             = m_maxTransforms * sizeof(XMFLOAT4X4);
		transform_buffer_desc.usageFlags       = vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress;
		transform_buffer_desc.memoryProperties = gpu::EMemoryProperties::eHostVisibleCoherent;

		m_transformSSBOs.resize(p_frames_in_flight);
		for (auto &ssbo: m_transformSSBOs)
			ssbo = m_resourceManager->createBuffer(transform_buffer_desc);
	}

	TransformSystem::~TransformSystem()
	{
		m_transformSSBOs.clear();
	}

	auto TransformSystem::createTransform() -> uint32
	{
		return m_freeTransformList.allocSlot();
	}

	auto TransformSystem::destroyTransform(uint32 p_transform_id) -> void
	{
		TST_ASSERT(p_transform_id < m_maxTransforms);
		m_freeTransformList.freeSlot(p_transform_id);
	}

	auto TransformSystem::updateTransform(uint32 p_transform_id, uint32 p_frame_index, XMMATRIX p_transform) -> void
	{
		TST_ASSERT(p_transform_id < m_maxTransforms);
		// More efficient
		gpu::BufferData *buffer_data{m_resourceManager->getBufferData(m_transformSSBOs[p_frame_index])};
		XMFLOAT4X4 *     mapped{static_cast<XMFLOAT4X4 *>(reinterpret_cast<void *>(reinterpret_cast<uint64>(buffer_data->mapped) + p_transform_id * sizeof(XMFLOAT4X4)))};
		XMStoreFloat4x4(mapped, p_transform);
	}
}
