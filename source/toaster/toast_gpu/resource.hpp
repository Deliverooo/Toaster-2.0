#pragma once

namespace toaster::gpu
{
	enum class EGPUResourceType
	{
		eUnknown,
		eUniformBuffer,
		eUniformBufferPFF,
		eStorageBuffer,
		eStorageBufferPFF,
		eTexture2D,
		eTexture3D
	};

	class TST_GPU_API IGPUResource
	{
	public:
		virtual ~IGPUResource() = default;

		virtual auto getResourceType() const -> EGPUResourceType = 0;
	};

	template<typename Type> concept GPUResource_c = std::derived_from<Type, IGPUResource>;
}
