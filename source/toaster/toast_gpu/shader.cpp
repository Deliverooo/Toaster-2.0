/*!
 * @file shader.cpp
 */
#include "shader.hpp"

#include "gl/gl_shader.hpp"

namespace toaster::gpu
{
	RefPtr<IShader> IShader::create(const String &p_name, const std::unordered_map<EShaderType, String> &p_shader_source_map)
	{
		return make_reference<GLShader>(p_name, p_shader_source_map);
	}
}
