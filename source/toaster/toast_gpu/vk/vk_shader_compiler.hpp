#pragma once

#include "vk_shader.hpp"
#include "toast_lib/ptr.hpp"

#include "toast_lib/io/filesystem.hpp"

namespace toaster::gpu
{
	class TST_GPU_API VKShaderCompiler final
	{
	public:
		static auto compileToBytecodeFromString(const String &p_source, vk::ShaderStageFlagBits p_stage) -> VKShader::Bytecode;
		static auto compileToBytecodeFromFilepath(const io::filesystem::Path &p_path, vk::ShaderStageFlagBits p_stage) -> VKShader::Bytecode;

		static auto compileToShaderFromStrings(VKLogicalDevice *p_device, const std::unordered_map<vk::ShaderStageFlagBits, String> &p_source_map,
											   const String &   p_name = "Compiled shader") -> RefPtr<VKShader>;
		static auto compileToShaderFromPaths(VKLogicalDevice *p_device, const std::unordered_map<vk::ShaderStageFlagBits, io::filesystem::Path> &p_path_map,
											 const String &   p_name = "Compiled shader") -> RefPtr<VKShader>;
	};
}
