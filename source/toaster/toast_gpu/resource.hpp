#pragma once

namespace toaster::gpu
{
	enum class EGPUResourceType
	{
		eUnknown,
		eUniformBuffer,
		eUniformBufferPFF,
		eTexture2D
	};

	class IGPUResource
	{
	public:
		virtual ~IGPUResource() = default;

		virtual EGPUResourceType getResourceType() const = 0;
	};
}
