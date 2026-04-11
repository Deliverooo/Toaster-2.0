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

			std::vector<PushConstantRange> pushConstantRanges;

			// // Ts just makes it easier to access the shader push constant buffers
			std::unordered_map<String, PushConstantBuffer> pushConstantBuffers;
		};

		using Bytecode              = std::vector<uint32>;
		using PipelineCreateInfoMap = std::unordered_map<vk::ShaderStageFlagBits, vk::PipelineShaderStageCreateInfo>;
		using BytecodeMap           = std::unordered_map<vk::ShaderStageFlagBits, Bytecode>;

		VKShader(VKGPUContext *p_ctx, const BytecodeMap &p_bytecode_map);
		VKGPUContext *getContext() const;

		[[nodiscard]] const PipelineCreateInfoMap &                  getPipelineShaderStageCreateInfoMap() const;
		[[nodiscard]] std::vector<vk::PipelineShaderStageCreateInfo> getPipelineShaderStageCreateInfos() const;
		[[nodiscard]] const vk::PipelineShaderStageCreateInfo &      getPipelineShaderStageCreateInfo(vk::ShaderStageFlagBits p_stage) const;

		[[nodiscard]] const BytecodeMap &getShaderBytecodeMap() const;
		[[nodiscard]] const Bytecode &   getShaderBytecode(vk::ShaderStageFlagBits p_stage) const;

		[[nodiscard]] std::vector<vk::DescriptorSetLayout> getDescriptorSetLayouts() const;
		[[nodiscard]] const vk::raii::DescriptorSetLayout &getDescriptorSetLayout(uint32 p_set_index) const;

		[[nodiscard]] const std::vector<DescriptorSet> &                    getReflectedShaderDescriptorSets() const;
		[[nodiscard]] const std::unordered_map<String, ShaderResource> &    getReflectedShaderResources() const;
		[[nodiscard]] const std::vector<PushConstantRange> &                getReflectedPushConstantRanges() const;
		[[nodiscard]] const std::unordered_map<String, PushConstantBuffer> &getReflectedPushConstantBuffers() const;

		[[nodiscard]] const std::vector<vk::DescriptorPoolSize> &getDescriptorPoolSizes(uint32 p_set_index) const;

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
