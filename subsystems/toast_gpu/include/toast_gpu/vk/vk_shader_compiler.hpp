#pragma once

#include "vk_shader.hpp"
#include "toast_lib/ptr.hpp"

#include "toast_lib/io/filesystem.hpp"

namespace toaster::gpu
{
	class TST_GPU_API ShaderCompiler
	{
		TST_GPU_OBJECT
	public:
		ShaderCompiler(VKLogicalDevice *p_device);

		[[nodiscard]] auto compileToBytecodeFromString(vk::ShaderStageFlagBits p_stage, const String &p_source) const -> VKShader::Bytecode;
		[[nodiscard]] auto compileToBytecodeFromFilepath(vk::ShaderStageFlagBits p_stage, const io::filesystem::Path &p_path) const -> VKShader::Bytecode;

		[[nodiscard]] auto compileToShaderFromStrings(const InitialiserList<const vk::ShaderStageFlagBits> &p_stages, const InitialiserList<const String> &p_sources,
													  const String &                                  p_name = "Compiled shader") const -> ShaderHandle;
		[[nodiscard]] auto compileToShaderFromPaths(const InitialiserList<const vk::ShaderStageFlagBits> &p_stages,
													const InitialiserList<const io::filesystem::Path> &   p_paths,
													const String &                                        p_name = "Compiled shader") const -> ShaderHandle;
	};
}
