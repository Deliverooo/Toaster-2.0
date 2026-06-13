#include "toast_render/shader_compiler.hpp"

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_hlsl.hpp>

#include "toast_render/render_context.hpp"

namespace toaster::render
{
	constexpr auto getVulkanShaderStage(EShaderStage p_stage) -> vk::ShaderStageFlagBits
	{
		switch (p_stage)
		{
			case EShaderStage::eNone: return static_cast<vk::ShaderStageFlagBits>(0);
			case EShaderStage::eVertex: return vk::ShaderStageFlagBits::eVertex;
			case EShaderStage::ePixel: return vk::ShaderStageFlagBits::eFragment;
			case EShaderStage::eCompute: return vk::ShaderStageFlagBits::eCompute;
		}

		return static_cast<vk::ShaderStageFlagBits>(0);
	}

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

	ShaderCompiler::ShaderCompiler(RenderContext &p_render_ctx) : m_renderCtx(&p_render_ctx)
	{
	}

	auto ShaderCompiler::compileToBytecodeFromString(const String &p_source, EShaderStage p_stage, EShaderLanguage p_shader_lang) const -> gpu::ShaderBytecode
	{
		const shaderc::Compiler compiler{};
		shaderc::CompileOptions compile_options{};
		compile_options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_4);

		compile_options.SetSourceLanguage((p_shader_lang == EShaderLanguage::eGLSL) ? shaderc_source_language_glsl : shaderc_source_language_hlsl);

		const shaderc::PreprocessedSourceCompilationResult preprocess_result{
			compiler.PreprocessGlsl(p_source, getShaderCShaderKind(getVulkanShaderStage(p_stage)), "I don't know", compile_options)
		};

		if (preprocess_result.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			LOG_ERROR("Failed to preprocess shader: {}", preprocess_result.GetErrorMessage());
		}

		const shaderc::SpvCompilationResult compilation_result{
			compiler.CompileGlslToSpv({preprocess_result.begin(), preprocess_result.end()}, getShaderCShaderKind(getVulkanShaderStage(p_stage)), "I still don't know",
									  compile_options)
		};

		if (compilation_result.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			LOG_ERROR("Failed to compile shader: {}", compilation_result.GetErrorMessage());
		}

		return {compilation_result.begin(), compilation_result.end()};
	}

	auto ShaderCompiler::compileToBytecodeFromPath(const io::filesystem::Path &p_path, EShaderStage p_stage, EShaderLanguage p_shader_lang) const -> gpu::ShaderBytecode
	{
		const String source{io::filesystem::readFile(p_path)};
		return compileToBytecodeFromString(source, p_stage, p_shader_lang);
	}

	auto ShaderCompiler::compileToShaderFromString(const String &  p_source, EShaderStage p_stage, EShaderStage p_next_stage,
												   EShaderLanguage p_shader_lang) const -> gpu::DynamicShaderHandle
	{
		gpu::ShaderBytecode      bytecode{compileToBytecodeFromString(p_source, p_stage, p_shader_lang)};
		gpu::DynamicShaderHandle out_shader{m_renderCtx->createGPURef<gpu::DynamicShader>(bytecode, getVulkanShaderStage(p_stage), getVulkanShaderStage(p_next_stage))};
		return out_shader;
	}

	auto ShaderCompiler::compileToShaderFromPath(const io::filesystem::Path &p_path, EShaderStage p_stage, EShaderStage p_next_stage,
												 EShaderLanguage             p_shader_lang) const -> gpu::DynamicShaderHandle
	{
		const String source{io::filesystem::readFile(p_path)};
		return compileToShaderFromString(source, p_stage, p_next_stage, p_shader_lang);
	}
}
