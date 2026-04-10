#pragma once

namespace toaster::gpu
{
	enum class EGPUResourceType
	{
		eUnknown,
		eUniformBuffer,
		eUniformBufferPFF,
		eTexture2D,
		eTexture3D
	};

	class IGPUResource
	{
	public:
		virtual ~IGPUResource() = default;

		virtual EGPUResourceType getResourceType() const = 0;
	};

	template<typename Type> concept GPUResource_c = std::derived_from<Type, IGPUResource>;
}
