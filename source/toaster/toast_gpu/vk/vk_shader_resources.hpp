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
		uint32                   size{0u};
		uint32                   binding{0u};
		String                   name{};
		vk::ShaderStageFlagBits  stage{vk::ShaderStageFlagBits::eAll};
	};

	struct DescriptorSet
	{
		// Set -> binding
		std::unordered_map<uint32, UniformBuffer> uniformBuffers;

		// Descriptor name -> vk::WriteDescriptorSet
		std::unordered_map<String, vk::WriteDescriptorSet> writeDescriptorSets;
	};
}
