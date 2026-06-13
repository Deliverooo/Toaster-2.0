#pragma once

#include "shader_library.hpp"
#include "toast_render.hpp"
#include "toast_lib/io/filesystem.hpp"

namespace toaster::render
{
	class RenderContext;

	enum class EShaderLanguage
	{
		eGLSL, eHLSL
	};

	enum class EShaderStage
	{
		// This will be used if the current shader stage is pixel and you don't have another one after it
		eNone,
		eVertex,
		ePixel,
		eCompute
	};

	class TST_RENDER_API ShaderCompiler
	{
		TST_RENDER_OBJECT
	public:
		ShaderCompiler(RenderContext &p_render_ctx);

		[[nodiscard]] auto compileToBytecodeFromString(const String &  p_source, EShaderStage p_stage,
													   EShaderLanguage p_shader_lang = EShaderLanguage::eHLSL) const -> gpu::ShaderBytecode;
		[[nodiscard]] auto compileToBytecodeFromPath(const io::filesystem::Path &p_path, EShaderStage p_stage,
													 EShaderLanguage             p_shader_lang = EShaderLanguage::eHLSL) const -> gpu::ShaderBytecode;

		[[nodiscard]] auto compileToShaderFromString(const String &p_source, EShaderStage p_stage, EShaderStage p_next_stage = EShaderStage::eNone,
													 EShaderLanguage p_shader_lang = EShaderLanguage::eHLSL) const -> gpu::DynamicShaderHandle;
		[[nodiscard]] auto compileToShaderFromPath(const io::filesystem::Path &p_path, EShaderStage p_stage, EShaderStage p_next_stage = EShaderStage::eNone,
												   EShaderLanguage             p_shader_lang = EShaderLanguage::eHLSL) const -> gpu::DynamicShaderHandle;
	};
}
