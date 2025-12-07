#pragma once

#include <filesystem>

#include "gpu/shader.hpp"

namespace tst
{
	struct ShaderReflectionData
	{
		std::vector<ShaderDescriptorSet>                           shaderDescriptorSets;
		std::unordered_map<std::string, ShaderResourceDeclaration> resources;
		std::unordered_map<std::string, ShaderBuffer>              constantBuffers;
		std::vector<ShaderPushConstantRange>                       pushConstantRanges;
	};

	class VulkanShader : public Shader
	{
	public:
		VulkanShader() = default;
		VulkanShader(const String &path, bool force_compile = false);
		~VulkanShader() override;

		void release();

		void reload(bool force_compile = false) override;
		void _reload(bool force_compile) override;

		const String &getName() const override { return m_name; }
		uint32        getHash() const override;

		void setShaderMacro(const String &macro_name, const String &value) override;

		const std::unordered_map<std::string, ShaderBuffer> &             getShaderBuffers() const override { return m_reflectionData.constantBuffers; }
		const std::unordered_map<std::string, ShaderResourceDeclaration> &getResources() const override;

		bool tryReadReflectionData(StreamReader *serializer);

		void serializeReflectionData(StreamWriter *serializer);

		void setReflectionData(const ShaderReflectionData &reflectionData);

		nvrhi::ShaderHandle                                     getHandle() const override { return m_shaderHandles.begin()->second; }
		nvrhi::ShaderHandle                                     getHandle(nvrhi::ShaderType type) const override;
		const std::map<nvrhi::ShaderType, nvrhi::ShaderHandle> &getHandles() const override { return m_shaderHandles; }

		const std::vector<ShaderDescriptorSet> &getShaderDescriptorSets() const { return m_reflectionData.shaderDescriptorSets; }
		bool                                    hasDescriptorSet(uint32_t set) const { return m_typeCounts.find(set) != m_typeCounts.end(); }

		const vk::WriteDescriptorSet *getDescriptorSet(const std::string &name, uint32 set = 0) const;

		nvrhi::BindingLayoutHandle        getDescriptorSetLayout(uint32_t set = 0) { return m_descriptorSetLayouts[set]; }
		const nvrhi::BindingLayoutVector &getAllDescriptorSetLayouts() const { return m_descriptorSetLayouts; }

	private:
		void loadAndCreateShaders(const std::map<nvrhi::ShaderType, std::vector<uint32> > &shaderData);
		void createDescriptors();

		std::map<nvrhi::ShaderType, nvrhi::ShaderHandle> m_shaderHandles;

		std::vector<vk::PipelineShaderStageCreateInfo> m_pipelineShaderStageCreateInfos;

		std::filesystem::path m_filePath;
		String                m_name;

		std::map<nvrhi::ShaderType, std::vector<uint32> > m_shaderData;
		ShaderReflectionData                              m_reflectionData;

		nvrhi::BindingLayoutVector m_descriptorSetLayouts;
		vk::DescriptorSet          m_descriptorSet;

		std::unordered_map<uint32, std::vector<VkDescriptorPoolSize> > m_typeCounts;

		bool m_disableOptimization = false;

		friend class VulkanShaderCompiler;
	};
}
