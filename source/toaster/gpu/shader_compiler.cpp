#include "shader_compiler.hpp"

#include <nvrhi/utils.h>
#include <shaderc/shaderc.hpp>

#include "logging.hpp"

namespace toaster::gpu::shader_compiler
{
	static shaderc_shader_kind shaderStageToShaderC(const nvrhi::ShaderType p_stage)
	{
		switch (p_stage)
		{
			case nvrhi::ShaderType::Vertex: { return shaderc_vertex_shader; }

			case nvrhi::ShaderType::Pixel: { return shaderc_fragment_shader; }

			case nvrhi::ShaderType::Compute: { return shaderc_compute_shader; }

			default:
			{
				break;
			}
		}
		return shaderc_vertex_shader;
	}

	bool compileShaderSource(const io::filesystem::Path &p_shader_path, const nvrhi::ShaderType p_shader_stage, std::vector<uint32> &p_out_binary)
	{
		static shaderc::Compiler s_compiler;

		const std::string stage_source = io::filesystem::readFile(p_shader_path);

		shaderc::CompileOptions compile_options;
		compile_options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_4);
		compile_options.SetWarningsAsErrors();
		compile_options.SetGenerateDebugInfo();
		compile_options.SetOptimizationLevel(shaderc_optimization_level_performance);

		const shaderc::CompilationResult module = s_compiler.CompileGlslToSpv(stage_source, shaderStageToShaderC(p_shader_stage), p_shader_path.string().c_str(),
																			  compile_options);

		if (module.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			LOG_ERROR("{} While compiling shader file: {} \nAt stage: {}", module.GetErrorMessage(), p_shader_path.string(),
					  nvrhi::utils::ShaderStageToString(p_shader_stage));
			return false;
		}

		p_out_binary = {module.begin(), module.end()};
		return true;
	}
}
