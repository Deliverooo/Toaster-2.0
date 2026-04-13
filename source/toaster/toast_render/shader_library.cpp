#include "shader_library.hpp"

#include "toast_lib/logging.hpp"

namespace toaster
{
	auto ShaderLibrary::add(const String &p_name, const RefPtr<gpu::VKShader> &p_shader) -> void
	{
		if (!m_shaders.contains(p_name))
			m_shaders[p_name] = p_shader;
		else
			LOG_ERROR("Attempted to add shader to library already containing one of that name {}", p_name);
	}

	auto ShaderLibrary::get(const String &p_name) const -> RefPtr<gpu::VKShader>
	{
		if (m_shaders.contains(p_name))
			return m_shaders.at(p_name);
		return nullptr;
	}
}
