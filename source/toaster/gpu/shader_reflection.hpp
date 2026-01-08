#pragma once

#include <unordered_map>
#include <nvrhi/nvrhi.h>
#include <vulkan/vulkan.hpp>
#include "shader_common.hpp"

namespace toaster::gpu::reflection
{
	enum class EShaderInputType
	{
		eUnknown,
		eUniformBuffer,
		eStorageBuffer,
		eImageSampler1D,
		eImageSampler2D,
		eImageSampler3D,
		eStorageImage1D,
		eStorageImage2D,
		eStorageImage3D
	};

	struct ShaderInputDeclaration
	{
		std::string      name;
		EShaderInputType type{EShaderInputType::eUnknown};
		uint32           set{0u};
		uint32           binding{0u};
		uint32           count{0u};
	};

	struct UniformBuffer
	{
		vk::DescriptorBufferInfo descriptor;
		uint32                   size{0u};
		uint32                   binding{0u};
		std::string              name;
		nvrhi::ShaderType        shaderStage{nvrhi::ShaderType::None};
	};

	struct ImageSampler
	{
		uint32            binding{0u};
		uint32            dimension{0u};
		uint32            arraySize{0u};
		std::string       name;
		nvrhi::ShaderType shaderStage{nvrhi::ShaderType::None};
	};

	struct DescriptorSet
	{
		std::unordered_map<uint32, UniformBuffer> uniformBuffers;
		std::unordered_map<uint32, ImageSampler>  imageSamplers;

		std::unordered_map<std::string, ShaderInputDeclaration> inputDeclarations;
	};

	struct ReflectionData
	{
		std::vector<reflection::DescriptorSet> descriptorSets;
	};

	void reflectShaderStage(nvrhi::ShaderType p_stage, ShaderBlob p_shader_binary, ReflectionData &p_out_reflection_data);
	void reflectShaderStage(nvrhi::ShaderType p_stage, const std::vector<uint32> &p_shader_binary, ReflectionData &p_out_reflection_data);
	void reflectShaderStage(nvrhi::ShaderType p_stage, const uint32 *p_blob_data, uint64 p_blob_size, ReflectionData &p_out_reflection_data);
}
