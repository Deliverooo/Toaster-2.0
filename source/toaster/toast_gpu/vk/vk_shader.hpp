#pragma once

#include <unordered_map>
#include <vulkan/vulkan_raii.hpp>

#include "vk_shader_resources.hpp"
#include "toast_lib/system_types.h"

namespace toaster::gpu
{
	class VKGPUContext;

	class VKShader
	{
	public:
		struct ReflectionData
		{
			std::vector<DescriptorSet> descriptorSets;

			// Ts just makes it easier to access the shader resources
			std::unordered_map<String, ShaderResource> resources; // Textures / images
		};

		using Bytecode              = std::vector<uint32>;
		using PipelineCreateInfoMap = std::unordered_map<vk::ShaderStageFlagBits, vk::PipelineShaderStageCreateInfo>;
		using BytecodeMap           = std::unordered_map<vk::ShaderStageFlagBits, Bytecode>;

		VKShader(VKGPUContext *p_ctx, const BytecodeMap &p_bytecode_map);

		const PipelineCreateInfoMap &                  getPipelineShaderStageCreateInfoMap() const;
		std::vector<vk::PipelineShaderStageCreateInfo> getPipelineShaderStageCreateInfos() const;
		const vk::PipelineShaderStageCreateInfo &      getPipelineShaderStageCreateInfo(vk::ShaderStageFlagBits p_stage) const;

		const BytecodeMap &getShaderBytecodeMap() const;
		const Bytecode &   getShaderBytecode(vk::ShaderStageFlagBits p_stage) const;

		std::vector<vk::DescriptorSetLayout> getDescriptorSetLayouts() const;
		const vk::raii::DescriptorSetLayout &getDescriptorSetLayout(uint32 p_set_index) const;

		const std::vector<DescriptorSet> &                getReflectedShaderDescriptorSets() const;
		const std::unordered_map<String, ShaderResource> &getReflectedShaderResources() const;

	private:
		void _reflect(vk::ShaderStageFlagBits p_stage, Bytecode p_bytecode);
		void _createDescriptors();

		VKGPUContext *m_ctx{nullptr};

		// Useful for the pipeline to use to access shader modules
		PipelineCreateInfoMap m_shaderCreateInfos;

		std::unordered_map<vk::ShaderStageFlagBits, vk::raii::ShaderModule> m_shaderModules{};

		// Stores the compiled shader code for each stage
		BytecodeMap m_shaderBytecodeMap;

		std::vector<vk::raii::DescriptorSetLayout> m_descriptorSetLayouts{};

		ReflectionData m_reflectionData{};

		// Set -> pool sizes
		std::unordered_map<uint32, std::vector<vk::DescriptorPoolSize> > m_poolSizes;
	};
}
