#pragma once

#include "vk_shader.hpp"
#include "toast_lib/ptr.hpp"

#include "toast_lib/io/filesystem.hpp"

namespace toaster::gpu::shader_compiler
{
	TST_GPU_API auto compileToBytecodeFromString(vk::ShaderStageFlagBits p_stage, const String &p_source) -> VKShader::Bytecode;
	TST_GPU_API auto compileToBytecodeFromFilepath(vk::ShaderStageFlagBits p_stage, const io::filesystem::Path &p_path) -> VKShader::Bytecode;

	TST_GPU_API auto compileToShaderFromStrings(VKLogicalDevice *p_device, InitialiserList<const vk::ShaderStageFlagBits> &p_stages,
												const InitialiserList<const String> &p_sources, const String &p_name = "Compiled shader") -> RefPtr<VKShader>;
	TST_GPU_API auto compileToShaderFromPaths(VKLogicalDevice *                                  p_device, const InitialiserList<const vk::ShaderStageFlagBits> &p_stages,
											  const InitialiserList<const io::filesystem::Path> &p_paths, const String &p_name = "Compiled shader") -> RefPtr<VKShader>;
}
