#pragma once

#include <concepts>
#include "toast_gpu.hpp"

#include <vulkan/vulkan.hpp>

#include "vk/vk_shader_resources.hpp"

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

	enum class EDescriptorType
	{
		eUnknown,
		eUniformBuffer,
		eStorageBuffer,
		eSampler2D,
		eSampler3D,
		eImage2D,
		eImage3D
	};

	struct WriteDescriptor
	{
		vk::WriteDescriptorSet wds{};
		// These are just pointers to the actual descriptors of the resource (e.g. The image view of a texture 2d)
		std::vector<void *> resourceHandles;
	};

	class TST_GPU_API IGPUResource
	{
	public:
		virtual ~IGPUResource() = default;

		[[nodiscard]] virtual auto getResourceType() const -> EGPUResourceType = 0;

		// See vk_descriptor_set_manager.cpp for usage
		virtual auto populateWriteDescriptor(vk::WriteDescriptorSet &p_write_descriptor, uint32 p_frame_index) -> void = 0;
		virtual auto getDescriptorResourceHandle(uint32 p_frame_index) -> void * = 0;
	};

	template<typename Type> concept GPUResource_c = std::derived_from<Type, IGPUResource>;

	#define TST_GPU_RESOURCE(__typename)\
		public:\
			[[nodiscard]] virtual auto getResourceType() const -> ::toaster::gpu::EGPUResourceType override\
														{ return ::toaster::gpu::EGPUResourceType::e##__typename; } private:

	TST_GPU_DEFINE_HANDLE(IGPUResource, GPUResource)

	struct TST_GPU_API DescriptorResource
	{
		std::vector<GPUResourceHandle> resources;
		EGPUResourceType               type{EGPUResourceType::eUnknown};

		DescriptorResource() = default;

		template<GPUResource_c TResource>
		DescriptorResource(const RefPtr<TResource> &p_resource) : resources(std::vector<GPUResourceHandle>(1, p_resource.template as<IGPUResource>())),
																  type(p_resource->getResourceType())
		{
		}

		// You should only use this for texture 2ds!
		template<GPUResource_c TResource>
		auto set(const RefPtr<TResource> &p_resource, uint32 p_index) -> void
		{
			type               = p_resource->getResourceType();
			resources[p_index] = p_resource.template as<IGPUResource>(); // Workaround to prevent wierd const related compile error
		}
	};

	constexpr auto getDescriptorType(vk::DescriptorType p_type) -> EDescriptorType
	{
		switch (p_type)
		{
			case vk::DescriptorType::eUniformBuffer: return EDescriptorType::eUniformBuffer;
			case vk::DescriptorType::eStorageBuffer: return EDescriptorType::eStorageBuffer;
			case vk::DescriptorType::eCombinedImageSampler:
			case vk::DescriptorType::eSampledImage:
				return EDescriptorType::eSampler2D;
			case vk::DescriptorType::eStorageImage:
				return EDescriptorType::eImage2D;
			default: return EDescriptorType::eUnknown;
		}
		return EDescriptorType::eUnknown;
	}

	inline auto descriptorTypeToString(EDescriptorType p_type) -> String
	{
		switch (p_type)
		{
			case EDescriptorType::eUnknown: return "Unknown";
			case EDescriptorType::eUniformBuffer: return "UniformBuffer";
			case EDescriptorType::eStorageBuffer: return "StorageBuffer";
			case EDescriptorType::eSampler2D: return "Sampler2D";
			case EDescriptorType::eSampler3D: return "Sampler3D";
			case EDescriptorType::eImage2D: return "Image2D";
			case EDescriptorType::eImage3D: return "Image3D";
		}
		return "";
	}

	constexpr auto getResourceType(vk::DescriptorType p_type) -> EGPUResourceType
	{
		switch (p_type)
		{
			case vk::DescriptorType::eUniformBuffer: return EGPUResourceType::eUniformBuffer;
			case vk::DescriptorType::eStorageBuffer: return EGPUResourceType::eStorageBuffer;
			case vk::DescriptorType::eCombinedImageSampler:
			case vk::DescriptorType::eSampledImage:
				return EGPUResourceType::eTexture2D;
			case vk::DescriptorType::eStorageImage:
				return EGPUResourceType::eStorageImage;
			default: return EGPUResourceType::eUnknown;
		}
		return EGPUResourceType::eUnknown;
	}

	constexpr auto getDescriptorImageSamplerType(vk::DescriptorType p_type, EImageDimension p_dimension) -> EDescriptorType
	{
		if (p_type == vk::DescriptorType::eSampledImage || p_type == vk::DescriptorType::eCombinedImageSampler)
		{
			switch (p_dimension)
			{
				case EImageDimension::e1D:
					break;
				case EImageDimension::e2D: return EDescriptorType::eSampler2D;
					break;
				case EImageDimension::e3D: return EDescriptorType::eSampler3D;
					break;
				case EImageDimension::eCube: return EDescriptorType::eSampler3D;
					break;
				default: break;
			}
		}
		else if (p_type == vk::DescriptorType::eStorageImage)
		{
			switch (p_dimension)
			{
				case EImageDimension::e1D:
					break;
				case EImageDimension::e2D: return EDescriptorType::eImage2D;
					break;
				case EImageDimension::e3D: return EDescriptorType::eImage3D;
					break;
				case EImageDimension::eCube: return EDescriptorType::eImage3D;
					break;
				default: break;
			}
		}

		return getDescriptorType(p_type);
	}
}
