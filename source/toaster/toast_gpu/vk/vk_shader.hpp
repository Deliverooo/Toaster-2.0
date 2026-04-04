#pragma once

#include <unordered_map>
#include <vulkan/vulkan_raii.hpp>
#include "toast_lib/system_types.h"

namespace toaster::gpu
{
	class VKGPUContext;

	class VKShader
	{
	public:
		using Bytecode              = std::vector<uint8>;
		using PipelineCreateInfoMap = std::unordered_map<vk::ShaderStageFlagBits, vk::PipelineShaderStageCreateInfo>;
		using BytecodeMap           = std::unordered_map<vk::ShaderStageFlagBits, Bytecode>;

		VKShader(VKGPUContext *p_ctx, const BytecodeMap &p_bytecode_map);

		const PipelineCreateInfoMap &                  getPipelineShaderStageCreateInfoMap() const;
		std::vector<vk::PipelineShaderStageCreateInfo> getPipelineShaderStageCreateInfos() const;
		const vk::PipelineShaderStageCreateInfo &      getPipelineShaderStageCreateInfo(vk::ShaderStageFlagBits p_stage) const;

		const BytecodeMap &getShaderBytecodeMap() const;
		const Bytecode &   getShaderBytecode(vk::ShaderStageFlagBits p_stage) const;

		const std::vector<vk::raii::DescriptorSet> &getDescriptorSets() const;
		const vk::raii::DescriptorSet &             getDescriptorSet(uint32 p_set_index) const;

		const std::vector<vk::raii::DescriptorSetLayout> &getDescriptorSetLayouts() const;
		const vk::raii::DescriptorSetLayout &             getDescriptorSetLayout(uint32 p_set_index) const;

	private:
		void _createDescriptors();

		VKGPUContext *m_ctx{nullptr};

		// Useful for the pipeline to use to access shader modules
		PipelineCreateInfoMap m_shaderCreateInfos;

		// Stores the compiled shader code for each stage
		BytecodeMap m_shaderBytecodeMap;

		std::vector<vk::raii::DescriptorSetLayout> m_descriptorSetLayouts;
		std::vector<vk::raii::DescriptorSet>       m_descriptorSets;
	};
}
