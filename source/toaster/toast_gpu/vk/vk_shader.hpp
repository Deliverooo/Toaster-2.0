#pragma once

#include <map>
#include <unordered_map>
#include <vulkan/vulkan_raii.hpp>

#include "vk_shader_resources.hpp"

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
		using PipelineCreateInfoMap = std::map<vk::ShaderStageFlagBits, vk::PipelineShaderStageCreateInfo>;
		using BytecodeMap           = std::map<vk::ShaderStageFlagBits, Bytecode>;

		VKShader(VKGPUContext *p_ctx, const BytecodeMap &p_bytecode_map, const String &p_name = "Unknown");
		[[nodiscard]] auto getContext() const -> VKGPUContext *;

		[[nodiscard]] auto getPipelineShaderStageCreateInfoMap() const -> const PipelineCreateInfoMap &;
		[[nodiscard]] auto getPipelineShaderStageCreateInfos() const -> std::vector<vk::PipelineShaderStageCreateInfo>;
		[[nodiscard]] auto getPipelineShaderStageCreateInfo(vk::ShaderStageFlagBits p_stage) const -> const vk::PipelineShaderStageCreateInfo &;

		[[nodiscard]] auto getShaderBytecodeMap() const -> const BytecodeMap &;
		[[nodiscard]] auto getShaderBytecode(vk::ShaderStageFlagBits p_stage) const -> const Bytecode &;

		[[nodiscard]] auto getDescriptorSetLayouts() const -> std::vector<vk::DescriptorSetLayout>;
		[[nodiscard]] auto getDescriptorSetLayout(uint32 p_set_index) const -> const vk::raii::DescriptorSetLayout &;

		[[nodiscard]] auto getReflectedShaderDescriptorSets() const -> const std::vector<DescriptorSet> &;
		[[nodiscard]] auto getReflectedShaderResources() const -> const std::unordered_map<String, ShaderResource> &;
		[[nodiscard]] auto getReflectedPushConstantRanges() const -> const std::vector<PushConstantRange> &;
		[[nodiscard]] auto getReflectedPushConstantBuffers() const -> const std::unordered_map<String, PushConstantBuffer> &;

		[[nodiscard]] auto getDescriptorPoolSizes(uint32 p_set_index) const -> const std::vector<vk::DescriptorPoolSize> &;

	private:
		auto _reflect(vk::ShaderStageFlagBits p_stage, Bytecode p_bytecode) -> void;
		auto _createDescriptors() -> void;

		VKGPUContext *m_ctx{nullptr};

		String m_name;

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
