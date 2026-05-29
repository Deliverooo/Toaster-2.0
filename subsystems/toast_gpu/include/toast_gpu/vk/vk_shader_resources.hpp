#pragma once

#include <unordered_map>
#include <vulkan/vulkan_raii.hpp>

#include "toast_lib/string.hpp"
#include "toast_lib/system_types.h"

namespace toaster::gpu
{
	struct UniformBuffer
	{
		vk::DescriptorBufferInfo descriptorInfo{};
		String                   name{};
		uint32                   size{0u};
		uint32                   binding{0u};
		vk::ShaderStageFlagBits  stage{vk::ShaderStageFlagBits::eAll};
	};

	struct StorageBuffer
	{
		vk::DescriptorBufferInfo descriptorInfo{};
		String                   name{};
		uint32                   size{0u};
		uint32                   binding{0u};
	};

	enum class EImageDimension
	{
		e1D,
		e2D,
		e3D,
		eCube
	};

	inline auto imageDimensionToString(EImageDimension p_dim) -> String
	{
		switch (p_dim)
		{
			case EImageDimension::e1D: return "1D";
			case EImageDimension::e2D: return "2D";
			case EImageDimension::e3D: return "3D";
			case EImageDimension::eCube: return "Cube";
		}
		return "";
	}

	struct ImageSampler
	{
		String                  name{};
		uint32                  binding{0u};
		uint32                  arraySize{0u};
		EImageDimension         dimension{EImageDimension::e2D};
		vk::ShaderStageFlagBits stage{vk::ShaderStageFlagBits::eAll};
	};

	struct DescriptorSet
	{
		// Set -> binding
		std::unordered_map<uint32, UniformBuffer> uniformBuffers;
		std::unordered_map<uint32, StorageBuffer> storageBuffers;
		std::unordered_map<uint32, ImageSampler>  imageSamplers;
		std::unordered_map<uint32, ImageSampler>  storageImages;
		std::unordered_map<uint32, ImageSampler>  separateImages;

		// Descriptor name -> vk::WriteDescriptorSet
		std::unordered_map<String, vk::WriteDescriptorSet> writeDescriptorSets;
	};

	struct PushConstantRange
	{
		vk::ShaderStageFlags stage{vk::ShaderStageFlagBits::eAll};
		uint32               offset{0u};
		uint32               size{0u};
	};

	struct PushConstant
	{
		String name{};
		uint32 size{0u};
		uint32 offset{0u};
	};

	struct PushConstantBuffer
	{
		std::unordered_map<String, PushConstant> pushConstants;
		String                                   name{};
		uint32                                   size{0u};
	};

	// A "Resource" is basically just another way of saying anything like a texture, sampler or combined image sampler
	struct ShaderResource
	{
		String name{};
		uint32 set{0u};
		uint32 binding{0u};
		uint32 arraySize{0u};
	};
}
