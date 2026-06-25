#include "toast_render/shader_compiler.hpp"

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_hlsl.hpp>

#include <wrl/client.h>

#include <dxc/dxcapi.h>
#pragma comment(lib, "d3dcompiler.lib")

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
			case EShaderStage::eTask: return vk::ShaderStageFlagBits::eTaskEXT;
			case EShaderStage::eMesh: return vk::ShaderStageFlagBits::eMeshEXT;
		}

		return static_cast<vk::ShaderStageFlagBits>(0);
	}

	constexpr auto getDxShaderStage(EShaderStage p_stage) -> WString
	{
		switch (p_stage)
		{
			case EShaderStage::eNone: return L"";
			case EShaderStage::eVertex: return L"vs_6_6";
			case EShaderStage::ePixel: return L"ps_6_6";
			case EShaderStage::eCompute: return L"cs_6_6";
			case EShaderStage::eTask: return L"ts_6_6";
			case EShaderStage::eMesh: return L"ms_6_6";
		}

		return L"";
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

	struct ShaderCompiler::Impl
	{
		Microsoft::WRL::ComPtr<IDxcUtils>          dxcUtils{nullptr};
		Microsoft::WRL::ComPtr<IDxcCompiler3>      dxcCompiler{nullptr};
		Microsoft::WRL::ComPtr<IDxcIncludeHandler> dxcIncludeHandler{nullptr};
	};

	ShaderCompiler::ShaderCompiler(RenderContext &p_render_ctx) : m_renderCtx(&p_render_ctx)
	{
		m_impl = new Impl{};

		DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_impl->dxcUtils));
		DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_impl->dxcCompiler));

		m_impl->dxcUtils->CreateDefaultIncludeHandler(&m_impl->dxcIncludeHandler);
	}

	ShaderCompiler::~ShaderCompiler()
	{
		delete m_impl;
	}

	auto ShaderCompiler::compileToBytecodeFromString(const String &p_source, EShaderStage p_stage, EShaderLanguage p_shader_lang) const -> gpu::ShaderBytecode
	{
		if (p_shader_lang == EShaderLanguage::eHLSL)
		{
			auto blob{_compileToDxBlob(p_source, p_stage)};

			uint64              size_in_words = (*(Microsoft::WRL::ComPtr<IDxcBlob> *) blob)->GetBufferSize() / sizeof(uint32);
			gpu::ShaderBytecode out_bytecode(size_in_words);

			const uint32_t *code = reinterpret_cast<const uint32 *>((*(Microsoft::WRL::ComPtr<IDxcBlob> *) blob)->GetBufferPointer());

			std::memcpy(out_bytecode.data(), code, size_in_words * sizeof(uint32));
			delete (Microsoft::WRL::ComPtr<IDxcBlob> *) blob;

			return out_bytecode;
		}
		else
		{
			const shaderc::Compiler compiler{};
			shaderc::CompileOptions compile_options{};
			compile_options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_4);
			compile_options.SetTargetSpirv(shaderc_spirv_version_1_6);

			compile_options.SetSourceLanguage(shaderc_source_language_glsl);

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

	auto ShaderCompiler::_compileToDxBlob(const String &p_source, EShaderStage p_stage) const -> void *
	{
		WString              stage_str{getDxShaderStage(p_stage)};
		std::vector<LPCWSTR> arguments{
			L"shader_thing.hlsl",
			L"-E",
			L"main",
			L"-T",
			stage_str.c_str(),
			L"-spirv",
			L"-fvk-use-dx-layout",
			L"-fvk-invert-y",
			L"-fspv-use-legacy-buffer-matrix-order",
			L"-fspv-target-env=vulkan1.3",
			L"-fspv-use-descriptor-heap",
			L"-fspv-extension=SPV_EXT_descriptor_heap",
			L"-fspv-extension=SPV_KHR_untyped_pointers",
			L"-fspv-extension=SPV_KHR_physical_storage_buffer",
		};

		DxcBuffer source_buffer;
		source_buffer.Ptr      = p_source.c_str();
		source_buffer.Size     = p_source.size();
		source_buffer.Encoding = DXC_CP_ACP;

		Microsoft::WRL::ComPtr<IDxcResult> compile_result;
		m_impl->dxcCompiler->Compile(&source_buffer, arguments.data(), arguments.size(), m_impl->dxcIncludeHandler.Get(), IID_PPV_ARGS(&compile_result));

		Microsoft::WRL::ComPtr<IDxcBlobUtf8> error_blob;
		compile_result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&error_blob), nullptr);
		if (error_blob && error_blob->GetStringLength() > 0)
		{
			LOG_ERROR("Failed to compile HLSL shader: {}", error_blob->GetStringPointer());
			TST_PERMA_ASSERT(false);
			std::abort();
			return nullptr;
		}

		HRESULT compile_status;
		compile_result->GetStatus(&compile_status);
		if (!SUCCEEDED(compile_status))
		{
			LOG_ERROR("Failed to compile HLSL shader");
			TST_PERMA_ASSERT(false);
			std::abort();
			return nullptr;
		}

		Microsoft::WRL::ComPtr<IDxcBlob> *shader_binary{new Microsoft::WRL::ComPtr<IDxcBlob>{}};
		compile_result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&*shader_binary), nullptr);

		return shader_binary;
	}
}
