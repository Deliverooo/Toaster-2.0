#pragma once

#include "../toast_gpu.hpp"

#include <map>
#include <unordered_map>
#include <vulkan/vulkan_raii.hpp>

#include "vk_shader_resources.hpp"

namespace toaster::gpu
{
	using ShaderBytecode = std::vector<uint32>;

	class TST_GPU_API VKShader
	{
		TST_GPU_OBJECT
	public:
		struct ReflectionData
		{
			std::vector<reflection::DescriptorSet> descriptorSets;

			// Ts just makes it easier to access the shader resources
			std::unordered_map<String, reflection::ShaderResource> resources; // Textures / images

			reflection::PushConstantRange              sharedPushConstantRange;
			std::vector<reflection::PushConstantRange> pushConstantRanges;

			// // Ts just makes it easier to access the shader push constant buffers
			std::unordered_map<String, reflection::PushConstantBuffer> pushConstantBuffers;
		};

		using Bytecode              = std::vector<uint32>;
		using PipelineCreateInfoMap = std::map<vk::ShaderStageFlagBits, vk::PipelineShaderStageCreateInfo>;
		using BytecodeMap           = std::map<vk::ShaderStageFlagBits, Bytecode>;

		VKShader(VKLogicalDevice *p_device, const BytecodeMap &p_bytecode_map, const String &p_name = "Unknown");
		VKShader(VKLogicalDevice *p_device, const InitialiserList<vk::ShaderStageFlagBits> &p_stages, const InitialiserList<Bytecode> &p_bytecodes,
				 const String &   p_name = "Unknown");
		VKShader(const VKShader &p_other) = delete;
		VKShader(VKShader &&p_other)      = delete;
		auto operator=(VKShader &&p_other) noexcept -> VKShader &;

		[[nodiscard]] auto getPipelineShaderStageCreateInfoMap() const -> const PipelineCreateInfoMap &;
		[[nodiscard]] auto getPipelineShaderStageCreateInfos() const -> std::vector<vk::PipelineShaderStageCreateInfo>;
		[[nodiscard]] auto getPipelineShaderStageCreateInfo(vk::ShaderStageFlagBits p_stage) const -> const vk::PipelineShaderStageCreateInfo &;

		[[nodiscard]] auto getShaderStageFlags() const -> vk::ShaderStageFlags;
		[[nodiscard]] auto getShaderBytecodeMap() const -> const BytecodeMap &;
		[[nodiscard]] auto getShaderBytecode(vk::ShaderStageFlagBits p_stage) const -> const Bytecode &;

		[[nodiscard]] auto getDescriptorSetLayouts() const -> std::vector<vk::DescriptorSetLayout>;
		[[nodiscard]] auto getDescriptorSetLayout(uint32 p_set_index) const -> const vk::raii::DescriptorSetLayout &;

		[[nodiscard]] auto getReflectedShaderDescriptorSets() const -> const std::vector<reflection::DescriptorSet> &;
		[[nodiscard]] auto getReflectedShaderResources() const -> const std::unordered_map<String, reflection::ShaderResource> &;
		[[nodiscard]] auto getReflectedPushConstantRanges() const -> const std::vector<reflection::PushConstantRange> &;
		[[nodiscard]] auto getReflectedPushConstantBuffers() const -> const std::unordered_map<String, reflection::PushConstantBuffer> &;

		[[nodiscard]] auto getDescriptorPoolSizes(uint32 p_set_index) const -> const std::vector<vk::DescriptorPoolSize> &;

	private:
		auto _reflect(vk::ShaderStageFlagBits p_stage, Bytecode p_bytecode) -> void;
		auto _createDescriptors() -> void;

		String m_name;

		// Useful for the pipeline to use to access shader modules
		PipelineCreateInfoMap m_shaderCreateInfos;

		std::unordered_map<vk::ShaderStageFlagBits, vk::raii::ShaderModule> m_shaderModules{};

		// Stores the compiled shader code for each stage
		BytecodeMap m_shaderBytecodeMap;

		std::vector<vk::raii::DescriptorSetLayout> m_descriptorSetLayouts;

		ReflectionData m_reflectionData{};

		// Set -> pool sizes
		std::unordered_map<uint32, std::vector<vk::DescriptorPoolSize> > m_poolSizes;
	};

	TST_GPU_DEFINE_HANDLE(VKShader, Shader)

	struct TST_GPU_API VKDynamicShader
	{
		TST_GPU_OBJECT
	public:
		VKDynamicShader() = default;
		auto operator=(VKDynamicShader &&p_other) noexcept -> VKDynamicShader &;
		VKDynamicShader(VKLogicalDevice *       p_device, const ShaderBytecode &p_bytecode, vk::ShaderStageFlagBits p_stage,
						vk::ShaderStageFlagBits p_next_stage = vk::ShaderStageFlagBits{0u});
		~VKDynamicShader();

		auto getShader() const -> vk::ShaderEXT;
		auto getStage() const -> vk::ShaderStageFlagBits;
		auto getNextStage() const -> vk::ShaderStageFlagBits;

	private:
		vk::raii::ShaderEXT     m_shader{nullptr};
		vk::ShaderStageFlagBits m_stage{};
		vk::ShaderStageFlagBits m_nextStage{};
	};

	TST_GPU_DEFINE_HANDLE(VKDynamicShader, DynamicShader)
}
