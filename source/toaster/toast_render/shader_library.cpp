#include "shader_library.hpp"

#include "toaster/toast_lib/logging.hpp"

namespace toaster
{
	void ShaderLibrary::add(const std::string &p_name, const RefPtr<gpu::Shader> &p_shader)
	{
		if (!m_shaders.contains(p_name))
		{
			m_shaders[p_name] = p_shader;
		}
		else
		{
			LOG_ERROR("Attempted to add shader to library already containing one of that name {}", p_name);
		}
	}

	RefPtr<gpu::Shader> ShaderLibrary::get(const std::string &p_name) const
	{
		if (m_shaders.contains(p_name))
		{
			return m_shaders.at(p_name);
		}
		return nullptr;
	}
}
