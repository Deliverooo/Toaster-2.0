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

	struct ImageSampler
	{
		String                  name{};
		uint32                  binding{0u};
		uint32                  arraySize{0u};
		vk::ShaderStageFlagBits stage{vk::ShaderStageFlagBits::eAll};
	};

	struct DescriptorSet
	{
		// Set -> binding
		std::unordered_map<uint32, UniformBuffer> uniformBuffers;
		std::unordered_map<uint32, ImageSampler>  imageSamplers;

		// Descriptor name -> vk::WriteDescriptorSet
		std::unordered_map<String, vk::WriteDescriptorSet> writeDescriptorSets;
	};

	struct PushConstantRange
	{
		vk::ShaderStageFlagBits stage{vk::ShaderStageFlagBits::eAll};
		uint32                  offset{0u};
		uint32                  size{0u};
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
