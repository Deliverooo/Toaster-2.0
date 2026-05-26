#pragma once

#include <concepts>
#include "toast_gpu.hpp"

#include <vulkan/vulkan.hpp>

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
		eTexture3D,
		eStorageImage
	};

	class TST_GPU_API IGPUResource
	{
	public:
		virtual ~IGPUResource() = default;

		[[nodiscard]] virtual auto getResourceType() const -> EGPUResourceType = 0;

		// See vk_descriptor_set_manager.cpp for usage
		virtual auto populateWriteDescriptor(vk::WriteDescriptorSet &p_write_descriptor) -> void = 0;
		virtual auto getDescriptorResourceHandle(uint32 p_frame_index = 0) -> void * = 0;
	};

	template<typename Type> concept GPUResource_c = std::derived_from<Type, IGPUResource>;

	#define TST_GPU_RESOURCE(__typename)\
		public:\
			[[nodiscard]] virtual auto getResourceType() const -> ::toaster::gpu::EGPUResourceType override\
														{ return ::toaster::gpu::EGPUResourceType::e##__typename; } private:

	TST_GPU_DEFINE_HANDLE(IGPUResource, GPUResource)
}
