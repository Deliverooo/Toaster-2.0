#pragma once

namespace toaster::gpu
{
	enum class EResourceType
	{
		eUniformBuffer,
		eUniformBufferSet,
		eStorageBuffer,
		eStorageBufferSet,
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
