#pragma once

#include "ptr.hpp"
#include "shader_common.hpp"

#include <map>
#include <string>

#include <glm/glm.hpp>

namespace toaster::gpu
{
	class Shader
	{
	public:
		static RefPtr<Shader> create(const std::string &p_name, const std::map<EShaderType, ShaderBlob> &p_shader_bytecode_map);
		static RefPtr<Shader> create(const std::string &p_name, const std::map<EShaderType, const char *> &p_shader_source_map);

		virtual ~Shader() = default;

		virtual void bind() const = 0;
		virtual void unbind() const = 0;

		virtual void setUniform(const std::string &p_name, float p_value) = 0;
		virtual void setUniform(const std::string &p_name, int p_value) = 0;
		virtual void setUniform(const std::string &p_name, uint32 p_value) = 0;
		virtual void setUniform(const std::string &p_name, const glm::vec2 &p_value) = 0;
		virtual void setUniform(const std::string &p_name, const glm::vec3 &p_value) = 0;
		virtual void setUniform(const std::string &p_name, const glm::vec4 &p_value) = 0;
		virtual void setUniform(const std::string &p_name, const glm::mat3 &p_value) = 0;
		virtual void setUniform(const std::string &p_name, const glm::mat4 &p_value) = 0;
	};
}
