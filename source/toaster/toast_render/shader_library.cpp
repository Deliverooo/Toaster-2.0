#include "shader_library.hpp"

#include "toast_lib/logging.hpp"

namespace toaster
{
	void ShaderLibrary::add(const std::string &p_name, const RefPtr<gpu::VKShader> &p_shader)
	{
		if (!m_shaders.contains(p_name))
			m_shaders[p_name] = p_shader;
		else
			LOG_ERROR("Attempted to add shader to library already containing one of that name {}", p_name);
	}

	RefPtr<gpu::VKShader> ShaderLibrary::get(const std::string &p_name) const
	{
		if (m_shaders.contains(p_name))
			return m_shaders.at(p_name);
		return nullptr;
	}
}
