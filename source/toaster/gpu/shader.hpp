#pragma once

#include <map>
#include <span>

#include "shader_common.hpp"
#include "shader_reflection.hpp"

namespace toaster::gpu
{
	class GPUContext;

	class Shader
	{
	public:
		Shader(GPUContext *p_ctx, const std::map<nvrhi::ShaderType, ShaderBlob> &p_shader_bytecode_map);
		~Shader();

		nvrhi::ShaderHandle getHandle() const;
		nvrhi::ShaderHandle getHandle(nvrhi::ShaderType p_shader_stage) const;

	private:
		GPUContext *m_gpuContext{nullptr};

		std::map<nvrhi::ShaderType, nvrhi::ShaderHandle> m_shaderHandles;

		std::map<nvrhi::ShaderType, ShaderBlob> m_shaderBytecodeMap;

		reflection::ReflectionData m_reflectionData;
	};
}
