#pragma once

#include "toast_render.hpp"
#include "toast_gpu/resource_manager.hpp"

#include "toast_math/math_matrix.hpp"

namespace toaster::rd
{
	using TransformID = uint32;

	// The transform system doesn't store any metadata to do with the transforms, so it doesn't need a pool.
	// To see how this could potentially be used in an ecs, goto toast_scene/components.hpp
	class TST_RENDER_API TransformSystem
	{
		TST_REGISTER_DEPENDENCY(gpu::ResourceManager, ResourceManager, resourceManager)
	public:
		TransformSystem(gpu::ResourceManager &p_resource_manager, uint32 p_max_transforms = 128u, uint32 p_frames_in_flight = 3u);
		~TransformSystem();

		auto createTransform() -> uint32;
		auto destroyTransform(uint32 p_transform_id) -> void;

		// Directly sets the transform into the requested slot.
		auto XM_CALLCONV updateTransform(uint32 p_transform_id, uint32 p_frame_index, XMMATRIX p_transform) -> void;

		auto getTransformBuffer(uint32 p_frame_index) const -> gpu::BufferHandle { return m_transformSSBOs[p_frame_index]; }

	private:
		FreelistAllocator<uint32>      m_freeTransformList;
		std::vector<gpu::BufferHandle> m_transformSSBOs;
		uint32                         m_maxTransforms{128u};
	};
}
