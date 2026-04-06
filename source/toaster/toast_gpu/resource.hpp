#pragma once

namespace toaster::gpu
{
	enum class EResourceType
	{
		eUniformBuffer,
		eUniformBufferPFF,
		eStorageBuffer,
		eStorageBufferPFF,
		eTexture2D,
		eTexture3D,
		eSampler
	};

	class IResource
	{
	public:
		virtual ~IResource() = default;

		virtual EResourceType getResourceType() const = 0;
	};
}
