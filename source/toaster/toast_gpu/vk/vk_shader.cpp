#include "vk_shader.hpp"

#include <ranges>

#include "vk_gpu_context.hpp"

namespace toaster::gpu
{
	VKShader::VKShader(VKGPUContext *p_ctx, const BytecodeMap &p_bytecode_map) : m_ctx(p_ctx), m_shaderBytecodeMap(p_bytecode_map)
	{
		for (auto &[stage, code]: p_bytecode_map)
		{
			const vk::raii::ShaderModule module = m_ctx->createShaderModule(code);

			vk::PipelineShaderStageCreateInfo &create_info = m_shaderCreateInfos[stage];
			create_info                                    = vk::PipelineShaderStageCreateInfo{};
			create_info.module                             = module;
			create_info.stage                              = stage;
			create_info.pName                              = "main";
		}
	}

	const VKShader::PipelineCreateInfoMap &VKShader::getPipelineShaderStageCreateInfoMap() const
	{
		return m_shaderCreateInfos;
	}

	std::vector<vk::PipelineShaderStageCreateInfo> VKShader::getPipelineShaderStageCreateInfos() const
	{
		std::vector<vk::PipelineShaderStageCreateInfo> result;
		for (const auto &info: m_shaderCreateInfos | std::views::values)
			result.emplace_back(info);
		return result;
	}

	const vk::PipelineShaderStageCreateInfo &VKShader::getPipelineShaderStageCreateInfo(vk::ShaderStageFlagBits p_stage) const
	{
		TST_ASSERT_MSG(m_shaderCreateInfos.contains(p_stage), "Stage is not present in shader create infos map");
		return m_shaderCreateInfos.at(p_stage);
	}

	const VKShader::BytecodeMap &VKShader::getShaderBytecodeMap() const
	{
		return m_shaderBytecodeMap;
	}

	const VKShader::Bytecode &VKShader::getShaderBytecode(vk::ShaderStageFlagBits p_stage) const
	{
		TST_ASSERT_MSG(m_shaderBytecodeMap.contains(p_stage), "Stage is not present in bytecode map");
		return m_shaderBytecodeMap.at(p_stage);
	}

	const std::vector<vk::raii::DescriptorSet> &VKShader::getDescriptorSets() const
	{
		return m_descriptorSets;
	}

	const vk::raii::DescriptorSet &VKShader::getDescriptorSet(uint32 p_set_index) const
	{
		TST_ASSERT_MSG(p_set_index < m_descriptorSets.size(), "Set index out of bounds");
		return m_descriptorSets.at(p_set_index);
	}

	const std::vector<vk::raii::DescriptorSetLayout> &VKShader::getDescriptorSetLayouts() const
	{
		return m_descriptorSetLayouts;
	}

	const vk::raii::DescriptorSetLayout &VKShader::getDescriptorSetLayout(uint32 p_set_index) const
	{
		TST_ASSERT_MSG(p_set_index < m_descriptorSetLayouts.size(), "Set index out of bounds");
		return m_descriptorSetLayouts.at(p_set_index);
	}

	void VKShader::_createDescriptors()
	{
	}
}
