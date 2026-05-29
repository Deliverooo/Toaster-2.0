#include "toast_gpu/vk/vk_shader_compiler.hpp"

#include "toast_lib/logging.hpp"
#include "toast_lib/toast_assert.h"

#include <shaderc/shaderc.hpp>

namespace toaster::gpu
{
	constexpr auto getShaderCShaderKind(vk::ShaderStageFlagBits p_stage) -> shaderc_shader_kind
	{
		switch (p_stage)
		{
			case vk::ShaderStageFlagBits::eVertex: return shaderc_vertex_shader;
			case vk::ShaderStageFlagBits::eTessellationControl: return shaderc_tess_control_shader;
			case vk::ShaderStageFlagBits::eTessellationEvaluation: return shaderc_tess_evaluation_shader;
			case vk::ShaderStageFlagBits::eGeometry: return shaderc_geometry_shader;
			case vk::ShaderStageFlagBits::eFragment: return shaderc_fragment_shader;
			case vk::ShaderStageFlagBits::eCompute: return shaderc_compute_shader;
			case vk::ShaderStageFlagBits::eRaygenKHR: return shaderc_raygen_shader;
			case vk::ShaderStageFlagBits::eAnyHitKHR: return shaderc_anyhit_shader;
			case vk::ShaderStageFlagBits::eClosestHitKHR: return shaderc_closesthit_shader;
			case vk::ShaderStageFlagBits::eMissKHR: return shaderc_miss_shader;
			case vk::ShaderStageFlagBits::eIntersectionKHR: return shaderc_intersection_shader;
			case vk::ShaderStageFlagBits::eCallableKHR: return shaderc_callable_shader;
			case vk::ShaderStageFlagBits::eTaskEXT: return shaderc_task_shader;
			case vk::ShaderStageFlagBits::eMeshEXT: return shaderc_mesh_shader;
			default: TST_ASSERT_MSG(false, "Unknown shader stage!");
				break;
		}
		TST_ASSERT_MSG(false, "Unknown shader stage!");
		return shaderc_vertex_shader;
	}

	ShaderCompiler::ShaderCompiler(VKLogicalDevice *p_device) : m_device(p_device)
	{
	}

	auto ShaderCompiler::compileToBytecodeFromString(vk::ShaderStageFlagBits p_stage, const String &p_source) const -> VKShader::Bytecode
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

	auto ShaderCompiler::compileToBytecodeFromFilepath(vk::ShaderStageFlagBits p_stage, const io::filesystem::Path &p_path) const -> VKShader::Bytecode
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

	auto ShaderCompiler::compileToShaderFromStrings(InitialiserList<const vk::ShaderStageFlagBits> &p_stages, const InitialiserList<const String> &p_sources,
													const String &                                  p_name) const -> ShaderHandle
	{
		VKShader::BytecodeMap bytecode_map;

		auto kit = p_stages.begin();
		auto vit = p_sources.begin();

		while (kit != p_stages.end() && vit != p_sources.end())
		{
			const VKShader::Bytecode bytecode{compileToBytecodeFromString(*kit, *vit)};
			if (bytecode.empty())
				return nullptr;
			bytecode_map[*kit] = bytecode;

			++kit;
			++vit;
		}
		return make_reference<VKShader>(m_device, bytecode_map, p_name);
	}

	auto ShaderCompiler::compileToShaderFromPaths(const InitialiserList<const vk::ShaderStageFlagBits> &p_stages,
												  const InitialiserList<const io::filesystem::Path> &   p_paths, const String &p_name) const -> ShaderHandle
	{
		VKShader::BytecodeMap bytecode_map;

		auto kit = p_stages.begin();
		auto vit = p_paths.begin();

		while (kit != p_stages.end() && vit != p_paths.end())
		{
			const VKShader::Bytecode bytecode{compileToBytecodeFromFilepath(*kit, *vit)};
			if (bytecode.empty())
				return nullptr;
			bytecode_map[*kit] = bytecode;

			++kit;
			++vit;
		}
		return make_reference<VKShader>(m_device, bytecode_map, p_name);
	}
}
