/*!
 * @file shader.cpp
 */
#include "shader.hpp"

#include "gl/gl_shader.hpp"

namespace toaster::gpu
{
	RefPtr<IShader> IShader::create(const std::string &p_name, const std::map<EShaderType, ShaderBlob> &p_shader_bytecode_map)
	{
		return make_reference<GLShader>(p_name, p_shader_bytecode_map);
	}

	RefPtr<IShader> IShader::create(const std::string &p_name, const std::map<EShaderType, const char *> &p_shader_source_map)
	{
		return make_reference<GLShader>(p_name, p_shader_source_map);
	}
}
