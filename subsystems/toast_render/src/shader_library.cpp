#include "toast_render/shader_library.hpp"

namespace toaster
{
	auto ShaderLibrary::add(const String &p_name, const gpu::ShaderHandle &p_shader) -> void
	{
		if (!m_shaders.contains(p_name))
			m_shaders[p_name] = p_shader;
		else
			LOG_ERROR("Attempted to add shader to library already containing one of that name {}", p_name);
	}

	auto ShaderLibrary::get(const String &p_name) const -> gpu::ShaderHandle
	{
		if (m_shaders.contains(p_name))
			return m_shaders.at(p_name);
		TST_PERMA_ASSERT(false);
		return nullptr;
	}

	auto DynamicShaderLibrary::add(const String &p_name, const gpu::DynamicShaderHandle &p_shader) -> void
	{
		if (!m_shaders.contains(p_name))
			m_shaders[p_name] = p_shader;
		else
			LOG_ERROR("Attempted to add shader to library already containing one of that name {}", p_name);
	}

	auto DynamicShaderLibrary::get(const String &p_name) const -> gpu::DynamicShaderHandle
	{
		if (m_shaders.contains(p_name))
			return m_shaders.at(p_name);
		TST_PERMA_ASSERT(false);
		return nullptr;
	}
}
