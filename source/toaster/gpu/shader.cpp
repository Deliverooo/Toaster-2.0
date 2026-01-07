#include "shader.hpp"

#include <ranges>

#include "gpu_context.hpp"

namespace toaster::gpu
{
	Shader::Shader(GPUContext *p_ctx, const std::map<nvrhi::ShaderType, ShaderBytecode> &p_shader_bytecode_map) : m_gpuContext(p_ctx)
	{
		m_shaderBytecodeMap = p_shader_bytecode_map;

		for (auto [shader_stage, binary]: m_shaderBytecodeMap)
		{
			nvrhi::ShaderDesc shader_desc{};
			shader_desc.shaderType = shader_stage;
			shader_desc.entryName  = "main";
			shader_desc.debugName  = "todo";

			m_shaderHandles[shader_stage] = m_gpuContext->getNVRHIDevice()->createShader(shader_desc, binary.data(), binary.size_bytes());

			reflection::reflectShaderStage(shader_stage, binary, m_reflectionData);
		}
	}

	Shader::~Shader()
	{
	}

	nvrhi::ShaderHandle Shader::getHandle() const
	{
		return m_shaderHandles.begin()->second;
	}

	nvrhi::ShaderHandle Shader::getHandle(nvrhi::ShaderType p_shader_stage) const
	{
		if (const auto it = m_shaderHandles.find(p_shader_stage); it != m_shaderHandles.end())
		{
			return it->second;
		}
		return nullptr;
	}
}
