#include "vk_shader_compiler.hpp"
#include <shaderc/shaderc.hpp>

#include "toast_lib/logging.hpp"
#include "toast_lib/toast_assert.h"

namespace toaster::gpu
{
	auto getShaderCShaderKind(vk::ShaderStageFlagBits p_stage) -> shaderc_shader_kind
	{
		switch (p_stage)
		{
			case vk::ShaderStageFlagBits::eVertex: return shaderc_vertex_shader;
				break;
			case vk::ShaderStageFlagBits::eTessellationControl: return shaderc_tess_control_shader;
				break;
			case vk::ShaderStageFlagBits::eTessellationEvaluation: return shaderc_tess_evaluation_shader;
				break;
			case vk::ShaderStageFlagBits::eGeometry: return shaderc_geometry_shader;
				break;
			case vk::ShaderStageFlagBits::eFragment: return shaderc_fragment_shader;
				break;
			case vk::ShaderStageFlagBits::eCompute: return shaderc_compute_shader;
				break;
			case vk::ShaderStageFlagBits::eRaygenKHR: return shaderc_raygen_shader;
				break;
			case vk::ShaderStageFlagBits::eAnyHitKHR: return shaderc_anyhit_shader;
				break;
			case vk::ShaderStageFlagBits::eClosestHitKHR: return shaderc_closesthit_shader;
				break;
			case vk::ShaderStageFlagBits::eMissKHR: return shaderc_miss_shader;
				break;
			case vk::ShaderStageFlagBits::eIntersectionKHR: return shaderc_intersection_shader;
				break;
			case vk::ShaderStageFlagBits::eCallableKHR: return shaderc_callable_shader;
				break;
			case vk::ShaderStageFlagBits::eTaskEXT: return shaderc_task_shader;
				break;
			case vk::ShaderStageFlagBits::eMeshEXT: return shaderc_mesh_shader;
				break;
			default: TST_ASSERT_MSG(false, "Unknown shader stage!");
				break;
		}
		TST_ASSERT_MSG(false, "Unknown shader stage!");
		return shaderc_vertex_shader;
	}

	auto VKShaderCompiler::compileToBytecodeFromString(const String &p_source, vk::ShaderStageFlagBits p_stage) -> VKShader::Bytecode
	{
		const shaderc::Compiler compiler{};
		shaderc::CompileOptions compile_options{};
		compile_options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_4);

		const shaderc::PreprocessedSourceCompilationResult preprocess_result{
			compiler.PreprocessGlsl(p_source, getShaderCShaderKind(p_stage), "I don't know", compile_options)
		};

		if (preprocess_result.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			LOG_ERROR("Failed to preprocess shader: {}", preprocess_result.GetErrorMessage());
		}

		const shaderc::SpvCompilationResult compilation_result{
			compiler.CompileGlslToSpv({preprocess_result.begin(), preprocess_result.end()}, getShaderCShaderKind(p_stage), "I still don't know", compile_options)
		};

		if (compilation_result.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			LOG_ERROR("Failed to compile shader: {}", compilation_result.GetErrorMessage());
		}

		return {compilation_result.begin(), compilation_result.end()};
	}

	auto VKShaderCompiler::compileToBytecodeFromFilepath(const io::filesystem::Path &p_path, vk::ShaderStageFlagBits p_stage) -> VKShader::Bytecode
	{
		String source{io::filesystem::readFile(p_path)};

		const shaderc::Compiler compiler{};
		shaderc::CompileOptions compile_options{};
		compile_options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_4);

		const shaderc::PreprocessedSourceCompilationResult preprocess_result{
			compiler.PreprocessGlsl(source, getShaderCShaderKind(p_stage), "I don't know", compile_options)
		};

		if (preprocess_result.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			LOG_ERROR("Failed to preprocess shader: {}", preprocess_result.GetErrorMessage());
			return {};
		}

		const shaderc::SpvCompilationResult compilation_result{
			compiler.CompileGlslToSpv({preprocess_result.begin(), preprocess_result.end()}, getShaderCShaderKind(p_stage), "I still don't know", compile_options)
		};

		if (compilation_result.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			LOG_ERROR("Failed to compile shader: {}", compilation_result.GetErrorMessage());
			return {};
		}

		return {compilation_result.begin(), compilation_result.end()};
	}

	auto VKShaderCompiler::compileToShaderFromStrings(VKGPUContext *p_ctx, const std::unordered_map<vk::ShaderStageFlagBits, String> &p_source_map,
													  const String &p_name) -> RefPtr<VKShader>
	{
		VKShader::BytecodeMap bytecode_map;
		for (const auto &[stage, source]: p_source_map)
		{
			const VKShader::Bytecode bytecode{compileToBytecodeFromString(source, stage)};
			if (bytecode.empty())
				return nullptr;
			bytecode_map[stage] = bytecode;
		}
		return make_reference<VKShader>(p_ctx, bytecode_map, p_name);
	}

	auto VKShaderCompiler::compileToShaderFromPaths(VKGPUContext *p_ctx, const std::unordered_map<vk::ShaderStageFlagBits, io::filesystem::Path> &p_path_map,
													const String &p_name) -> RefPtr<VKShader>
	{
		VKShader::BytecodeMap bytecode_map;
		for (const auto &[stage, path]: p_path_map)
		{
			const VKShader::Bytecode bytecode{compileToBytecodeFromFilepath(path, stage)};
			if (bytecode.empty())
				return nullptr;
			bytecode_map[stage] = bytecode;
		}
		return make_reference<VKShader>(p_ctx, bytecode_map, p_name);
	}
}
