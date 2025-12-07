#pragma once
#include "vulkan_shader_compiler.hpp"

namespace tst
{
	class VulkanShaderCache
	{
	public:
		static nvrhi::ShaderType hasChanged(RefPtr<VulkanShaderCompiler> shader);

	private:
		static void serialize(const std::map<String, std::map<nvrhi::ShaderType, StageData> > &shaderCache);
		static void deserialize(std::map<String, std::map<nvrhi::ShaderType, StageData> > &shaderCache);
	};
}
