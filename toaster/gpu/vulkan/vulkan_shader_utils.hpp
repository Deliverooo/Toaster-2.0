#pragma once

#include "gpu/shader.hpp"

#include <nvrhi/nvrhi.h>

#include <shaderc/shaderc.h>

namespace tst::shader_utils
{
	inline nvrhi::ShaderType preprocessorStageToShaderStage(const std::string_view stage)
	{
		if (stage == "vert")
			return nvrhi::ShaderType::Vertex;
		if (stage == "frag")
			return nvrhi::ShaderType::Pixel;
		if (stage == "comp")
			return nvrhi::ShaderType::Compute;
		TST_ASSERT(false);
		return nvrhi::ShaderType::None;
	}

	inline std::string_view shaderStageToShaderMacro(const nvrhi::ShaderType stage)
	{
		if (stage == nvrhi::ShaderType::Vertex)
			return "__VERTEX_STAGE__";
		if (stage == nvrhi::ShaderType::Pixel)
			return "__FRAGMENT_STAGE__";
		if (stage == nvrhi::ShaderType::Compute)
			return "__COMPUTE_STAGE__";
		TST_ASSERT(false);
		return "";
	}

	inline EShaderLanguage shaderLangFromExtension(const std::string_view type)
	{
		if (type == ".glsl")
			return EShaderLanguage::eGLSL;
		if (type == ".hlsl")
			return EShaderLanguage::eHLSL;

		TST_ASSERT(false);

		return EShaderLanguage::eGLSL;
	}

	inline shaderc_shader_kind shaderStageToShaderC(const nvrhi::ShaderType stage)
	{
		switch (stage)
		{
			case nvrhi::ShaderType::Vertex: return shaderc_vertex_shader;
			case nvrhi::ShaderType::Pixel: return shaderc_fragment_shader;
			case nvrhi::ShaderType::Compute: return shaderc_compute_shader;
		}
		TST_ASSERT(false);
		return {};
	}

	inline const char *shaderStageCachedFileExtension(const nvrhi::ShaderType stage, bool debug)
	{
		if (debug)
		{
			switch (stage)
			{
				case nvrhi::ShaderType::Vertex: return ".cached_vulkan_debug.vert";
				case nvrhi::ShaderType::Pixel: return ".cached_vulkan_debug.frag";
				case nvrhi::ShaderType::Compute: return ".cached_vulkan_debug.comp";
			}
		}
		else
		{
			switch (stage)
			{
				case nvrhi::ShaderType::Vertex: return ".cached_vulkan.vert";
				case nvrhi::ShaderType::Pixel: return ".cached_vulkan.frag";
				case nvrhi::ShaderType::Compute: return ".cached_vulkan.comp";
			}
		}
		TST_ASSERT(false);
		return "";
	}

	#ifdef TST_PLATFORM_WINDOWS
	inline const wchar_t *HLSLShaderProfile(const nvrhi::ShaderType stage)
	{
		switch (stage)
		{
			case nvrhi::ShaderType::Vertex: return L"vs_6_0";
			case nvrhi::ShaderType::Pixel: return L"ps_6_0";
			case nvrhi::ShaderType::Compute: return L"cs_6_0";
		}
		TST_ASSERT(false);
		return L"";
	}
	#else
	inline const char *HLSLShaderProfile(const nvrhi::ShaderType stage)
	{
		switch (stage)
		{
			case nvrhi::ShaderType::Vertex: return "vs_6_0";
			case nvrhi::ShaderType::Pixel: return "ps_6_0";
			case nvrhi::ShaderType::Compute: return "cs_6_0";
		}
		TST_ASSERT(false);
		return "";
	}
	#endif
}
